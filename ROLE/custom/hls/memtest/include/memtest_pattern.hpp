
#ifndef _ROLE_MEMTEST_PATTERN_H_
#define _ROLE_MEMTEST_PATTERN_H_

#include <stdio.h>
#include <iostream>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>
#include "../../../../../HOST/custom/memtest/languages/cplusplus/include/config.h"

#include "network.hpp"

using namespace hls;

#define FAULT_INJECTION // macro for fault injection insertion

#define FSM_PROCESSING_WAIT_FOR_META 0
#define FSM_PROCESSING_PCKT_PROC 1
#define FSM_PROCESSING_STOP 2
#define FSM_PROCESSING_START 3
#define FSM_PROCESSING_WRITE 4
#define FSM_PROCESSING_READ 5
#define FSM_PROCESSING_OUTPUT 6
#define FSM_PROCESSING_OUTPUT_2 7
#define FSM_PROCESSING_OUTPUT_3 8
#define FSM_PROCESSING_OUTPUT_4 9
#define FSM_PROCESSING_OUTPUT_5 10
#define FSM_PROCESSING_CONTINUOUS_RUN 11

#define ProcessingFsmType uint8_t


#define LOCAL_MEM_WORD_SIZE 512
#define LOCAL_MEM_ADDR_SIZE 20
#define MEMTEST_ADDRESS_BITWIDTH 40
#define MEMTEST_ITERATION_BITWIDTH 16


typedef ap_uint<LOCAL_MEM_WORD_SIZE>  local_mem_word_t;
typedef ap_uint<MEMTEST_ADDRESS_BITWIDTH>  local_mem_addr_t; 
#define LOCAL_MEM_ADDR_SIZE_NON_BYTE_ADDRESSABLE 16 // TODO: to parametrize better
typedef ap_uint<LOCAL_MEM_ADDR_SIZE_NON_BYTE_ADDRESSABLE>  local_mem_addr_non_byteaddressable_t; 
#define LOCAL_MEM_ADDR_OFFSET (LOCAL_MEM_WORD_SIZE/8) //byte addres offset
#define LOCAL_MEM_WORD_BYTE_SIZE (LOCAL_MEM_WORD_SIZE/8) //byte size of a local mem word

#define MEMTEST_ADDRESS_HIGH_BIT NETWORK_WORD_BIT_WIDTH-1 // 63
#define MEMTEST_ADDRESS_LOW_BIT NETWORK_WORD_BIT_WIDTH-MEMTEST_ADDRESS_BITWIDTH //64-40 = 24

#define MEMTEST_ITERATIONS_HIGH_BIT MEMTEST_ADDRESS_LOW_BIT-1 // 23
#define MEMTEST_ITERATIONS_LOW_BIT  MEMTEST_ITERATIONS_HIGH_BIT+1-MEMTEST_ITERATION_BITWIDTH // 

#define MAX_ITERATION_COUNT 10
const unsigned int max_proc_fifo_depth = MAX_ITERATION_COUNT;

template<typename Tevent=bool, const unsigned int counter_width=32, const unsigned int maximum_counter_value_before_reset=4000000>
void pCountClockCycles(
hls::stream<Tevent> &sOfEnableCCIncrement,
hls::stream<Tevent> &sOfResetCounter, 
hls::stream<Tevent> &sOfGetTheCounter,
hls::stream<ap_uint<counter_width> > &oSClockCounter)
{

  static ap_uint<counter_width> internal_counter = 0;
  static bool pop_the_counter = false;
#pragma HLS reset variable=internal_counter
#pragma HLS reset variable=pop_the_counter
//giving priority to the pop
  if(!sOfGetTheCounter.empty()){
    pop_the_counter = sOfGetTheCounter.read();
  }
  if (pop_the_counter && !oSClockCounter.full())
  {
    oSClockCounter.write(internal_counter);
    pop_the_counter=false;
  }
  if(!sOfResetCounter.empty()){
    bool reset_or_not = sOfResetCounter.read();
    if (reset_or_not)
    {
      internal_counter = 0;
    }
  }
  if(!sOfEnableCCIncrement.empty()){
    bool increment = sOfEnableCCIncrement.read();
    if (increment)
    {
      if(internal_counter==maximum_counter_value_before_reset){
        internal_counter=1;
      }else{
        internal_counter++;
      }
#ifndef __SYNTHESIS__
      printf("DEBUG pCountClockCycles counter value = %s\n", internal_counter.to_string().c_str());
#endif //__SYNTHESIS__
      
    }
  }
}

/*****************************************************************************
 * @brief pPortAndDestionation - Setup the port and the destination rank.
 *
 * @param[in]  pi_rank
 * @param[in]  pi_size
 * @param[out] sDstNode_sig
 * @param[out] po_rx_ports
 *
 * @return Nothing.
 ******************************************************************************/
void pPortAndDestionation(
    ap_uint<32>             *pi_rank,
    ap_uint<32>             *pi_size,
    stream<NodeId>          &sDstNode_sig,
    ap_uint<32>             *po_rx_ports
    )
{
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS INLINE off
  //-- STATIC VARIABLES (with RESET) ------------------------------------------
  static PortFsmType port_fsm = FSM_WRITE_NEW_DATA;
#pragma HLS reset variable=port_fsm

  switch(port_fsm)
  {
    default:
    case FSM_WRITE_NEW_DATA:
        //Triangle app needs to be reset to process new rank
        if(!sDstNode_sig.full())
        {
          NodeId dst_rank = (*pi_rank + 1) % *pi_size;
          printf("rank: %d; size: %d; \n", (int) *pi_rank, (int) *pi_size);
          sDstNode_sig.write(dst_rank);
          *po_rx_ports = 0x0; //init the value
          port_fsm = FSM_DONE;
        }
        break;
    case FSM_DONE:
        *po_rx_ports = 0x1; //currently work only with default ports...
        break;
  }

}
//const char * lorem_ipsum_pattern = "Lorem ipsum dolor sit amet, consectetur adipiscing elit,sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.Feugiat in fermentum posuere urna.Purus semper eget duis at tellus at urna condimentum mattis.Euismod nisi porta lorem mollis aliquam ut porttitor leo a. Dolor purus non enim praesent elementum facilisis. Ultrices gravida dictum fusce ut placerat orci.Egestas integer eget aliquet nibh. Pharetra pharetra massa massa ultricies mi quis hendrerit dolor.Ut tortor pretium viverra suspendisse potenti. Mattis aliquam faucibus purus in massa tempor nec feugiat. Tellus integer feugiat scelerisque varius morbi enim nunc faucibus a.Ipsum dolor sit amet consectetur adipiscing elit ut aliquam. Lacinia at quis risus sed vulputate odio ut enim blandit. Facilisi morbi tempus iaculis urna id.Elementum facilisis leo vel fringilla est ullamcorper eget.Dolor sed viverra ipsum nunc.Non tellus orci ac auctor augue mauris augue neque. Sit amet facilisis magna etiam tempor orci eu lobortis.Pellentesque diam volutpat commodo sed egestas egestas fringilla phasellus.Ac felis donec et odio pellentesque diam volutpat. In ornare quam viverra orci sagittis eu volutpat. Rhoncus mattis rhoncus urna neque. Proin sagittis nisl rhoncus mattis rhoncus urna neque viverra justo.Morbi blandit cursus risus at ultrices mi tempus imperdiet nulla.Eu sem integer vitae justo eget magna fermentum iaculis eu.Risus quis varius quam quisque id diam vel. Sem viverra aliquet eget sit amet tellus cras.Ut tristique et egestas quis ipsum suspendisse ultrices gravida. Semper risus in hendrerit gravida rutrum quisque non tellus orci.Massa placerat duis ultricies lacus sed turpis. Sem fringilla ut morbi tincidunt augue interdum velit.Vitae congue mauris rhoncus aenean vel elit.Eu lobortis elementum nibh tellus molestie nunc non blandit massa.Nibh sit amet commodo nulla facilisi nullam vehicula. In fermentum et sollicitudin ac orci phasellus egestas tellus rutrum. Ut sem nulla pharetra diam sit amet nisl suscipit.Quis enim lobortis scelerisque fermentum dui. Lacus suspendisse faucibus interdum posuere lorem ipsum dolor sit amet.Enim tortor at auctor urna nunc. Sed arcu non odio euismod lacinia at. Natoque penatibus et magnis dis parturient montes nascetur.Mus mauris vitae ultricies leo integer malesuada. Viverra tellus in hac habitasse platea dictumst vestibulum rhoncus est.Facilisis gravida neque convallis a cras semper. Imperdiet nulla malesuada pellentesque elit eget gravida. Risus nec feugiat in fermentum posuere.Turpis tincidunt id aliquet risus. Felis imperdiet proin fermentum leo vel orci porta.Tristique senectus et netus et malesuada fames ac turpis egestas.Arcu dictum varius duis at consectetur lorem. Tristique magna sit amet purus gravida quis blandit.Sapien nec sagittis aliquam malesuada bibendum arcu vitae elementum.Commodo viverra maecenas accumsan lacus. Arcu ac tortor dignissim convallis aenean et.Integer feugiat scelerisque varius morbi enim nunc. Tellus integer feugiat scelerisque varius morbi enim nunc faucibus.In egestas erat imperdiet sed euismod nisi. Cursus metus aliquam eleifend mi in nulla posuere sollicitudin aliquam.Parturient montes nascetur ridiculus mus mauris vitae ultricies. Eros donec ac odio tempor orci.Enim facilisis gravida neque convallis a cras semper auctor. Odio pellentesque diam volutpat commodo.Volutpat commodo sed egestas egestas fringilla phasellus.Neque egestas congue quisque egestas diam in arcu cursus euismod.Adipiscing diam donec adipiscing tristique risus. Quis auctor elit sed vulputate mi sit amet mauris commodo.Quis blandit turpis cursus in hac habitasse platea dictumst. Praesent semper feugiat nibh sed pulvinar.Lorem mollis aliquam ut porttitor leo a. Sollicitudin nibh sit amet commodo nulla facilisi nullam vehicula ipsum.Tortor dignissim convallis aenean et tortor at risus viverra adipiscing. Ac turpis egestas sed tempus urna et.Lectus proin nibh nisl condimentum id venenatis a condimentum vitae. Tempus egestas sed sed risus pretium quam.Tortor dignissim convallis aenean et tortor. Aliquet nec ullamcorper sit amet risus.Faucibus in ornare quam viverra orci sagittis eu volutpat odio. Quis eleifend quam adipiscing vitae proin sagittis nisl.Vestibulum rhoncus est pellentesque elit ullamcorper dignissim cras.Morbi tincidunt augue interdum velit euismod in pellentesque massa placerat.Pellentesque massa placerat duis ultricies lacus sed turpis tincidunt.Sit amet nisl purus in mollis. Posuere sollicitudin aliquam ultrices sagittis orci a.\0";

template<typename Tin, typename Tout>
Tout genNextFibonacciNumber(Tin curr, Tin prev){
#pragma HLS INLINE
  return static_cast<Tout>(curr + prev);
}

template<typename ADDR_T, unsigned int sequenceDim, typename BIGWORD_T, typename SMALLWORD_T, unsigned int smallWordDim>
void genFibonacciNumbers(ADDR_T curr, BIGWORD_T * outBigWord){
//#pragma HLS INLINE off

// REQUIREMENT: must be a multiple
  //assert( (BIGWORD_T%SMALLWORD_T)==0);

  SMALLWORD_T currentFibonacciNumber = static_cast<SMALLWORD_T>(curr);
  SMALLWORD_T prevFibonacciNumber = currentFibonacciNumber + 1;
  SMALLWORD_T nextFibonacciNumber = genNextFibonacciNumber<ADDR_T,SMALLWORD_T>(currentFibonacciNumber,prevFibonacciNumber);

  gen_sequence_loop: for (unsigned int i = 0; i < sequenceDim; i++)
  {
#pragma HLS PIPELINE
    //printf("MEMTEST PATTERN %d %d\n", smallWordDim*(i+1)-1,smallWordDim*i);
    // printf("MEMTEST PATTERN Next fibo number %u\n", nextFibonacciNumber.to_string());
    //   std'::cout << "MEMTEST PATTERN Next fibo number " << nextFibonacciNumber << std::endl;
    (*outBigWord).range(smallWordDim*(i+1)-1,smallWordDim*i)=nextFibonacciNumber;
    prevFibonacciNumber=currentFibonacciNumber;
    currentFibonacciNumber=nextFibonacciNumber;
    nextFibonacciNumber=genNextFibonacciNumber<ADDR_T,SMALLWORD_T>(currentFibonacciNumber,prevFibonacciNumber);
    #ifndef __SYNTHESIS__
    std::cout << "MEMTEST PATTERN prev " << prevFibonacciNumber << " curr " << currentFibonacciNumber << "  next " << nextFibonacciNumber << std::endl;
    #endif //__SYNTHESIS__
  }
  

}


template<typename ADDR_T, unsigned int sequenceDim, typename BIGWORD_T, typename SMALLWORD_T, unsigned int smallWordDim>
void genXoredSequentialNumbers(ADDR_T curr, BIGWORD_T * outBigWord){
#pragma HLS INLINE off

// REQUIREMENT: must be a multiple
  //assert( (BIGWORD_T%SMALLWORD_T)==0);

  SMALLWORD_T currentNumber = static_cast<SMALLWORD_T>(curr);
  SMALLWORD_T nextNumber = (currentNumber+1) xor 1;
  SMALLWORD_T prevNumber = currentNumber;

  gen_sequence_loop: for (unsigned int i = 0; i < sequenceDim; i++)
  {
#pragma HLS PIPELINE
    //printf("MEMTEST PATTERN %d %d\n", smallWordDim*(i+1)-1,smallWordDim*i);
    // printf("MEMTEST PATTERN Next fibo number %u\n", nextFibonacciNumber.to_string());
    //   std'::cout << "MEMTEST PATTERN Next fibo number " << nextFibonacciNumber << std::endl;
    (*outBigWord).range(smallWordDim*(i+1)-1,smallWordDim*i)=nextNumber;
    prevNumber = currentNumber;
    currentNumber = nextNumber;
    nextNumber = (nextNumber + 1 ) xor i;
    // #ifndef __SYNTHESIS__
    //  std::cout << "MEMTEST PATTERN prev " << prevNumber << " curr " << currentNumber << "  next " << nextNumber << " \t";
   //  std::cout << "MEMTEST PATTERN curr " << currentNumber << " \t";
    // #endif //__SYNTHESIS__
  }
 //    std::cout << std::endl;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////TODO: //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 * @brief THIS processing the data once recieved a start command
 *
 * @param[in]  sRxpToProcp_Data
 * @param[out] sProcpToTxp_Data
 * @param[in]  start_stop
 *
 * @return Nothing.
 ******************************************************************************/
 template<typename Tevent=bool, const unsigned int counter_width=32>
 void pTHISProcessingData(
  stream<NetworkWord>                              &sRxpToProcp_Data,
  stream<NetworkWord>                              &sProcpToTxp_Data,
  stream<NetworkMetaStream>                        &sRxtoProc_Meta,
  stream<NetworkMetaStream>                        &sProctoTx_Meta,
  bool *                                           start_stop,
  hls::stream<Tevent>                              &sOutEnableCCIncrement,
  hls::stream<Tevent>                              &sOutResetCounter, 
  hls::stream<Tevent>                              &sOutGetTheCounter,
  hls::stream<ap_uint<counter_width>>              &sInClockCounter
){
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma  HLS INLINE off
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord    netWord;
    NetworkWord    outNetWord;
    static NetworkMetaStream  outNetMeta = NetworkMetaStream();;
    static ProcessingFsmType processingFSM  = FSM_PROCESSING_WAIT_FOR_META;

    static ap_uint<16> max_iterations;
    static local_mem_addr_t first_faulty_address;
#pragma HLS reset variable=first_faulty_address
    static ap_uint<32> faulty_addresses_cntr;
#pragma HLS reset variable=faulty_addresses_cntr
    static local_mem_addr_t max_address_under_test; // byte addressable;
#pragma HLS reset variable=max_address_under_test
#pragma HLS reset variable=outNetMeta
#pragma HLS reset variable=processingFSM
static size_t bytes_sent_for_tx =0;


// static hls::stream<ap_uint<32>> sFaulty_addresses_cntr("sFaulty_addresses_cntr");
// #pragma HLS STREAM variable=sFaulty_addresses_cntr depth=max_proc_fifo_depth dim=1
// static hls::stream<local_mem_addr_t> sFirst_faulty_address("sFirst_faulty_address");
// #pragma HLS STREAM variable=sFirst_faulty_address depth=max_proc_fifo_depth dim=1


    static local_mem_word_t local_under_test_memory [LOCAL_MEM_ADDR_SIZE];
    static local_mem_addr_non_byteaddressable_t local_mem_addr_non_byteaddressable;
    static local_mem_addr_t curr_address_under_test;
    static int writingCounter;
    static int readingCounter;
    static ap_uint<32> testCounter;
    static ap_uint<counter_width> reading_cntr = 0;
    static ap_uint<counter_width> writing_cntr = 0;


    local_mem_word_t testingVector;
    local_mem_word_t goldenVector;


#pragma HLS reset variable=local_under_test_memory
#pragma HLS reset variable=local_mem_addr_non_byteaddressable
#pragma HLS reset variable=curr_address_under_test
#pragma HLS reset variable=address_under_test
#pragma HLS reset variable=writingCounter
#pragma HLS reset variable=readingCounter
#pragma HLS reset variable=testCounter
#pragma HLS reset variable=testingVector

//assuming that whnever I send a start I must complete the run and then restart unless a stop
// or stopping once done with the run
    switch(processingFSM)
    {
      case FSM_PROCESSING_WAIT_FOR_META:// this is done only to ensure that producing a data for each meta
      //assuming also the shell is able to take care of sending multiple meta if needed
      printf("DEBUG proc FSM, I am in the WAIT_FOR_META state\n");
      //resetting once per test suite
      max_address_under_test = 0;
      max_iterations=0;
      bytes_sent_for_tx = 0;

      if ( !sRxtoProc_Meta.empty()  && !sProctoTx_Meta.full())
      {
        outNetMeta = sRxtoProc_Meta.read();
        sProctoTx_Meta.write(outNetMeta);
        processingFSM = FSM_PROCESSING_PCKT_PROC;
      }
      break;

      case FSM_PROCESSING_PCKT_PROC:
      printf("DEBUG proc FSM, I am in the PROCESSING_PCKT_PROC state\n");
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////TODO: adding logic for saving the meta and sending continuously at the end of each test????
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //TODO: it seems that without a continuous meta each end of packet will stop the TX path
      if ( !sRxpToProcp_Data.empty() && !sProcpToTxp_Data.full())
      {
        netWord = sRxpToProcp_Data.read();
        switch (netWord.tdata.range(MEMTEST_COMMANDS_HIGH_BIT,MEMTEST_COMMANDS_LOW_BIT))
        {
        case TEST_INVLD_CMD:
          /* FWD an echo of the invalid*/
          printf("DEBUG processing the packet with invalid cmd\n");
          sProcpToTxp_Data.write(netWord);
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;
        case TEST_STOP_CMD:
          /* call with stop (never started), unset, fwd the stop */
          printf("DEBUG processing the packet with stop cmd\n");
          outNetWord.tdata=TEST_STOP_CMD;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 1;
          sProcpToTxp_Data.write(outNetWord);
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;    
        default:
          /* Execute*/
         // printf("DEBUG processing the packet with the address within cmd\n");
          max_address_under_test = netWord.tdata(MEMTEST_ADDRESS_HIGH_BIT,MEMTEST_ADDRESS_LOW_BIT);
          max_iterations = netWord.tdata.range(MEMTEST_ITERATIONS_HIGH_BIT,MEMTEST_ITERATIONS_LOW_BIT);
    #ifndef __SYNTHESIS__
          printf("DEBUG processing the packet with the address %s within cmd %s\n", max_address_under_test.to_string().c_str(), max_iterations.to_string().c_str());
    #endif //__SYNTHESIS__
          processingFSM = FSM_PROCESSING_START;
          break;
        }
      }
      break;

      case FSM_PROCESSING_START:
        printf("DEBUG proc FSM, I am in the START state\n");
        curr_address_under_test = 0; 
        first_faulty_address = 0; 
        faulty_addresses_cntr = 0;
        local_mem_addr_non_byteaddressable = 0;
        readingCounter=0;
        writingCounter=0;
        testCounter = 0;
        processingFSM = FSM_PROCESSING_WRITE;
        break;

      case FSM_PROCESSING_CONTINUOUS_RUN:
        testCounter += 1;
        curr_address_under_test = 0; 
        local_mem_addr_non_byteaddressable = 0;
        readingCounter=0;
        writingCounter=0;
        faulty_addresses_cntr = 0;
        // check if need another meta to send out!
        // if already over the MTU size, or with a command (stop case) or with another iteration I need to send out another meta
        if(*start_stop && testCounter < max_iterations){ //stopping conditions: either a Stop or the maximum iterations
    #ifndef __SYNTHESIS__
          printf("DEBUG processing continuous run (still run, iter %s) max iters: %s\n",testCounter.to_string().c_str(),max_iterations.to_string().c_str());
    #endif //__SYNTHESIS__
          processingFSM = FSM_PROCESSING_WRITE;
          if(bytes_sent_for_tx >= PACK_SIZE){
            sProctoTx_Meta.write(outNetMeta);
            std::cout <<  "DEBUG: writing an additional meta with bytes sent equal to " << bytes_sent_for_tx << std::endl;
            bytes_sent_for_tx = 0;
          }
          break;
        }else{
    #ifndef __SYNTHESIS__
          printf("DEBUG processing continuous run (stop the run at iter %s) max iters: %s \n",testCounter.to_string().c_str(),max_iterations.to_string().c_str());
    #endif //__SYNTHESIS__
          
          //FWD the stop and signal the end of the packet with the iteration of the tests performed
          outNetWord.tdata.range(MEMTEST_COMMANDS_HIGH_BIT,MEMTEST_COMMANDS_LOW_BIT)=TEST_ENDOFTESTS_CMD;
          outNetWord.tdata.range(NETWORK_WORD_BIT_WIDTH-1,MEMTEST_COMMANDS_HIGH_BIT+1)=testCounter;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 1;
          sProcpToTxp_Data.write(outNetWord);
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;
        }

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////Custom User Logic here//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

      case FSM_PROCESSING_WRITE:
      printf("DEBUG proc FSM, I am in the WRITE state\n");
    #ifndef __SYNTHESIS__
      std::cout << "DEBUG I have to test " << max_address_under_test<< std::endl;
      std::cout << "DEBUG current addr " << local_mem_addr_non_byteaddressable<< std::endl;
    #endif //__SYNTHESIS__

    //if not written all the needed memory cells write
      if (curr_address_under_test < max_address_under_test)// max is byte addressable!
      {
        printf("DEBUG WRITE FSM, writing the memory with counter %d\n",writingCounter);
        genXoredSequentialNumbers<local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32, local_mem_word_t, ap_uint<32>,32>(local_mem_addr_non_byteaddressable, local_under_test_memory+local_mem_addr_non_byteaddressable);
        #ifdef FAULT_INJECTION
        //TODO:  place for control fault injection
        if(testCounter >= 2 && local_mem_addr_non_byteaddressable > 0){
            local_under_test_memory[local_mem_addr_non_byteaddressable] &= static_cast<local_mem_word_t>(0);
        }
        #endif // FAULT_INJECTION
        local_mem_addr_non_byteaddressable += 1;
        curr_address_under_test += LOCAL_MEM_ADDR_OFFSET;
        writingCounter += 1;
        if(!sOutEnableCCIncrement.full()){
          sOutEnableCCIncrement.write(true);
        }
        processingFSM = FSM_PROCESSING_WRITE;
        break;

      } else{
        printf("DEBUG WRITE FSM, done with the write\n");
        local_mem_addr_non_byteaddressable = 0;
        curr_address_under_test = 0;
        if(!sOutGetTheCounter.full()){
          sOutGetTheCounter.write(true);
        }
        if(!sOutResetCounter.full()){
          sOutResetCounter.write(true);
        }
        processingFSM = FSM_PROCESSING_READ;
        break;
      }

    case FSM_PROCESSING_READ:
        printf("DEBUG proc FSM, I am in the READ state\n");
      //if not read all the needed memory cells read
        if (!sInClockCounter.empty())
        {
          writing_cntr = sInClockCounter.read();
        }
      
        if (curr_address_under_test < max_address_under_test) // max is byte addressable!
        {
          printf("DEBUG READ FSM, reading the memory\n");
          genXoredSequentialNumbers<local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32, local_mem_word_t, ap_uint<32>,32>(local_mem_addr_non_byteaddressable, &goldenVector);
          testingVector=local_under_test_memory[local_mem_addr_non_byteaddressable];
          golden_comparison: for (int i = 0; i < LOCAL_MEM_ADDR_OFFSET; ++i)
          {
    #pragma HLS UNROLL
            if (testingVector.range((i+1)*8-1,i*8) != goldenVector.range((i+1)*8-1,i*8))
            {
              if (faulty_addresses_cntr == 0) //first fault
              {
                first_faulty_address = i+curr_address_under_test; //save the fault address
              }
              faulty_addresses_cntr += 1; //increment the fault counter
            } 
          //std::cout << "First faulty address " << first_faulty_address << " address faulty cntr " << faulty_addresses_cntr << std::endl;
          }
          curr_address_under_test += LOCAL_MEM_ADDR_OFFSET; //next test
          local_mem_addr_non_byteaddressable += 1;
          readingCounter += 1;
          if(!sOutEnableCCIncrement.full()){
            sOutEnableCCIncrement.write(true);
          }
          processingFSM = FSM_PROCESSING_READ;
          break;
        } else{ //done with the reads
          printf("DEBUG READ FSM, done with the read\n");
          std::cout << "First faulty address " << first_faulty_address << " address faulty cntr " << faulty_addresses_cntr << std::endl;
          if(!sOutGetTheCounter.full()){
            sOutGetTheCounter.write(true);
          }
          if(!sOutResetCounter.full()){
            sOutResetCounter.write(true);
          }
          processingFSM = FSM_PROCESSING_OUTPUT;
          break;
        }
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////Custom output management Logic here/////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
      case FSM_PROCESSING_OUTPUT:
      printf("DEBUG processing the output of a run\n");
      if (!sInClockCounter.empty())
      {
          reading_cntr = sInClockCounter.read();
      }
      
      if(!sProcpToTxp_Data.full()){
          outNetWord.tdata = max_address_under_test;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          sProcpToTxp_Data.write(outNetWord);
          bytes_sent_for_tx += 8;
          processingFSM = FSM_PROCESSING_OUTPUT_2;
          // outNetWord.tdata=faulty_addresses_cntr;
          // outNetWord.tkeep = 0xFF;
          // outNetWord.tlast = 0;
          // sProcpToTxp_Data.write(outNetWord);
          // outNetWord.tdata=first_faulty_address;
          // outNetWord.tkeep = 0xFF;
          // outNetWord.tlast = 0;
          // sProcpToTxp_Data.write(outNetWord);
          // bytes_sent_for_tx += 8 * 3;
          // processingFSM = FSM_PROCESSING_CONTINUOUS_RUN;
      }
      break;

        ///////
      ////TODO: right now not used
      ///////

      case FSM_PROCESSING_OUTPUT_2:
      printf("DEBUG processing the output of a run part 3\n");
      
      if(!sProcpToTxp_Data.full()){
          outNetWord.tdata=faulty_addresses_cntr;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          sProcpToTxp_Data.write(outNetWord);
          bytes_sent_for_tx += 8 ;
          processingFSM = FSM_PROCESSING_OUTPUT_3;
      }
      break;
      case FSM_PROCESSING_OUTPUT_3:
      printf("DEBUG processing the output of a run part 4\n");
      
      if(!sProcpToTxp_Data.full()){
          outNetWord.tdata=first_faulty_address;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          sProcpToTxp_Data.write(outNetWord);
          bytes_sent_for_tx += 8;
          processingFSM = FSM_PROCESSING_OUTPUT_4;
      }
      break;
      case FSM_PROCESSING_OUTPUT_4:
      printf("DEBUG processing the output of a run part 5\n");
      
      if(!sProcpToTxp_Data.full()){
          outNetWord.tdata.range(64-1,64-32) = writing_cntr;
          outNetWord.tdata.range(32-1,0) = reading_cntr;
          //memcpy(outNetWord.tdata.range(), &writing_cntr, 4);
          //memcpy(outNetWord.tdata.range(), &reading_cntr, 4);
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          sProcpToTxp_Data.write(outNetWord);
          bytes_sent_for_tx += 8;
          processingFSM = FSM_PROCESSING_CONTINUOUS_RUN;
      }
      break;


    // default:
    //   processingFSM = FSM_PROCESSING_WAIT_FOR_META;
    //   break;
    }
};

#endif //_ROLE_MEMTEST_PATTERN_H_
