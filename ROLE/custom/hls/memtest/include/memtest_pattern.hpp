
#ifndef _ROLE_MEMTEST_PATTERN_H_
#define _ROLE_MEMTEST_PATTERN_H_

#include <stdio.h>
#include <iostream>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>

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
#define FSM_PROCESSING_CONTINUOUS_RUN 7
#define ProcessingFsmType uint8_t


#define LOCAL_MEM_WORD_SIZE 512
#define LOCAL_MEM_ADDR_SIZE 20
#define MEMTEST_ADDRESS_BITWIDTH 40

typedef ap_uint<LOCAL_MEM_WORD_SIZE>  local_mem_word_t;
typedef ap_uint<MEMTEST_ADDRESS_BITWIDTH>  local_mem_addr_t; 
#define LOCAL_MEM_ADDR_SIZE_NON_BYTE_ADDRESSABLE 16 // TODO: to parametrize better
typedef ap_uint<LOCAL_MEM_ADDR_SIZE_NON_BYTE_ADDRESSABLE>  local_mem_addr_non_byteaddressable_t; 
#define LOCAL_MEM_ADDR_OFFSET (LOCAL_MEM_WORD_SIZE/8) //byte addres offset
#define LOCAL_MEM_WORD_BYTE_SIZE (LOCAL_MEM_WORD_SIZE/8) //byte size of a local mem word

#define MEMTEST_ADDRESS_HIGH_BIT NETWORK_WORD_BIT_WIDTH-1
#define MEMTEST_ADDRESS_LOW_BIT NETWORK_WORD_BIT_WIDTH-MEMTEST_ADDRESS_BITWIDTH

#define MAX_ITERATION_COUNT 10

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
//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 * @brief Function that emulate a start and stop functionality similar to an ap_hs interface
 * and test the memory
 *
 * @param[in] max_address_under_test
 * @param[in]  start_stop
 * @param[out]  sFaulty_addresses_cntr
 * @param[out]  sFirst_faulty_address
 *
 * @return Nothing.
 ******************************************************************************/
void memoryTestProcessingOnly(bool * start_stop, local_mem_addr_t * max_address_under_test, hls::stream<ap_uint<32>> &sFaulty_addresses_cntr, hls::stream<local_mem_addr_t> &sFirst_faulty_address){

    static local_mem_word_t local_under_test_memory [LOCAL_MEM_ADDR_SIZE];

    static local_mem_addr_non_byteaddressable_t local_mem_addr_non_byteaddressable;
    static local_mem_addr_t curr_address_under_test;
    static int writingCounter;
    static int readingCounter;
    static ap_uint<32> testCounter;
    static ProcessingFsmType processingInternalFSM  = FSM_PROCESSING_STOP;

    local_mem_addr_t first_faulty_address;
    ap_uint<32> faulty_addresses_cntr;
    local_mem_word_t testingVector;
    local_mem_word_t goldenVector;


#pragma HLS reset variable=processingInternalFSM
#pragma HLS reset variable=local_under_test_memory
#pragma HLS reset variable=local_mem_addr_non_byteaddressable
#pragma HLS reset variable=curr_address_under_test
#pragma HLS reset variable=address_under_test
#pragma HLS reset variable=writingCounter
#pragma HLS reset variable=readingCounter
#pragma HLS reset variable=testCounter
#pragma HLS reset variable=testingVector


  switch (processingInternalFSM)
  {
  case FSM_PROCESSING_START:
    printf("DEBUG proc FSM, I am in the START state\n");
    testCounter += 1;
    if(*start_stop && testCounter <= MAX_ITERATION_COUNT){
      processingInternalFSM = FSM_PROCESSING_WRITE;
    } else {
      processingInternalFSM = FSM_PROCESSING_STOP;
    }

  case FSM_PROCESSING_WRITE:
    printf("DEBUG proc FSM, I am in the WRITE state\n");
  #ifndef __SYNTHESIS__
    std::cout << "DEBUG I have to test " << max_address_under_test << std::endl;
  #endif //__SYNTHESIS__

  //if not written all the needed memory cells write
    if (local_mem_addr_non_byteaddressable < max_address_under_test)
    {
      printf("DEBUG WRITE FSM, writing the memory with counter %d\n",writingCounter);
      genXoredSequentialNumbers<local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32, local_mem_word_t, ap_uint<32>,32>(local_mem_addr_non_byteaddressable, local_under_test_memory+local_mem_addr_non_byteaddressable);
      #ifdef FAULT_INJECTION
      if(testCounter > 2 && local_mem_addr_non_byteaddressable > 0){
          local_under_test_memory[local_mem_addr_non_byteaddressable] &= 0;
      }
      #endif // FAULT_INJECTION
      local_mem_addr_non_byteaddressable += 1;
      curr_address_under_test += LOCAL_MEM_ADDR_OFFSET;
      writingCounter += 1;
    } else{
      printf("DEBUG WRITE FSM, done with the write\n");
      local_mem_addr_non_byteaddressable = 0;
      curr_address_under_test = 0;
      processingInternalFSM = FSM_PROCESSING_READ;
    }

  case FSM_PROCESSING_READ:
    printf("DEBUG proc FSM, I am in the READ state\n");
  //if not read all the needed memory cells read
    if (local_mem_addr_non_byteaddressable < max_address_under_test)
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
      }
      curr_address_under_test += LOCAL_MEM_ADDR_OFFSET; //next test
      local_mem_addr_non_byteaddressable += 1;

    } else{ //done with the reads
      printf("DEBUG READ FSM, done with the read\n");
      sFaulty_addresses_cntr.write(faulty_addresses_cntr);
      sFirst_faulty_address.write(first_faulty_address);
      processingInternalFSM = FSM_PROCESSING_READ;
    }

  default:
    testCounter = 0;
    //resetting each execution to the 0 address
    curr_address_under_test = 0; 
    first_faulty_address = 0; 
    faulty_addresses_cntr = 0;
    local_mem_addr_non_byteaddressable = 0;
    readingCounter=0;
    writingCounter=0;
    if(*start_stop && testCounter <= MAX_ITERATION_COUNT){
      processingInternalFSM = FSM_PROCESSING_START;
    } else {
      break;
    }
  }
    
  

};


/*****************************************************************************
 * @brief THIS processing the data once recieved a start command
 *
 * @param[in]  sRxpToProcp_Data
 * @param[out] sProcpToTxp_Data
 * @param[in]  start_stop
 *
 * @return Nothing.
 ******************************************************************************/
 void pTHISProcessingData(
  stream<NetworkWord>                              &sRxpToProcp_Data,
  stream<NetworkWord>                              &sProcpToTxp_Data,
  stream<NetworkMetaStream>                        &sRxtoProc_Meta,
  stream<NetworkMetaStream>                        &sProctoTx_Meta,
  bool *                                             start_stop
){
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma  HLS INLINE off
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord    netWord;
    NetworkWord    outNetWord;
    static NetworkMetaStream  outNetMeta = NetworkMetaStream();;
    static ProcessingFsmType processingFSM  = FSM_PROCESSING_WAIT_FOR_META;


    static local_mem_addr_t first_faulty_address;
#pragma HLS reset variable=first_faulty_address
    static ap_uint<32> faulty_addresses_cntr;
#pragma HLS reset variable=faulty_addresses_cntr
    static local_mem_addr_t max_address_under_test; // byte addressable;
#pragma HLS reset variable=max_address_under_test
#pragma HLS reset variable=outNetMeta
#pragma HLS reset variable=processingFSM


static hls::stream<ap_uint<32>> sFaulty_addresses_cntr("sFaulty_addresses_cntr");
#pragma HLS STREAM variable=sFaulty_addresses_cntr depth=MAX_ITERATION_COUNT dim=1
static hls::stream<local_mem_addr_t> sFirst_faulty_address("sFirst_faulty_address");
#pragma HLS STREAM variable=sFirst_faulty_address depth=MAX_ITERATION_COUNT dim=1

//assuming that whnever I send a start I must complete the run and then restart unless a stop
// or stopping once done with the run
    switch(processingFSM)
    {
      case FSM_PROCESSING_WAIT_FOR_META:
      //assuming also the shell is able to take care of sending multiple meta if needed
      printf("DEBUG proc FSM, I am in the WAIT_FOR_META state\n");
      //resetting once per test suite
      max_address_under_test = 0; 
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
      if ( !sRxpToProcp_Data.empty() && !sProcpToTxp_Data.full())
      {
        netWord = sRxpToProcp_Data.read();
        switch (netWord.tdata)
        {
        case TEST_INVLD_CMD:
          /* FWD an echo of the invalid*/
          sProcpToTxp_Data.write(netWord);
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;
        case TEST_STOP_CMD:
          /* call with stop, unset, fwd the stop */
          memoryTestProcessingOnly(start_stop, &max_address_under_test, sFaulty_addresses_cntr, sFirst_faulty_address );
          sProcpToTxp_Data.write(netWord);
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;    
        default:
          /* Execute*/
          max_address_under_test = netWord.tdata(MEMTEST_ADDRESS_BITWIDTH-1,0);
          memoryTestProcessingOnly(start_stop, &max_address_under_test, sFaulty_addresses_cntr, sFirst_faulty_address );
          processingFSM = FSM_PROCESSING_OUTPUT;
          //break;
        }
      }
      break;

      case FSM_PROCESSING_CONTINUOUS_RUN:
        if(*start_stop){
          memoryTestProcessingOnly(start_stop, &max_address_under_test, sFaulty_addresses_cntr, sFirst_faulty_address);
          processingFSM = FSM_PROCESSING_OUTPUT;
          break;
        }else{
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;
        }

      case FSM_PROCESSING_OUTPUT:
      if(!sProcpToTxp_Data.full()){
          /* read results and forward*/
          outNetWord.tdata = max_address_under_test;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
	        sProcpToTxp_Data.write(outNetWord);
          while (!sFaulty_addresses_cntr.empty()&& !sFirst_faulty_address.empty())
          {
            faulty_addresses_cntr = sFaulty_addresses_cntr.read();
            outNetWord.tdata=faulty_addresses_cntr;
            outNetWord.tkeep = 0xFF;
            outNetWord.tlast = 0;
	          sProcpToTxp_Data.write(outNetWord);
            first_faulty_address = sFirst_faulty_address.read();
            outNetWord.tdata=first_faulty_address;
            outNetWord.tkeep = 0xFF;
            outNetWord.tlast = 0;
	          sProcpToTxp_Data.write(outNetWord);
          }
          outNetWord.tdata=TEST_STOP_CMD;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 1;
	        sProcpToTxp_Data.write(outNetWord);
          processingFSM = FSM_PROCESSING_CONTINUOUS_RUN;
      }
      break;

    default:
      processingFSM = FSM_PROCESSING_WAIT_FOR_META;
      break;
    }
};


#endif //_ROLE_MEMTEST_PATTERN_H_
