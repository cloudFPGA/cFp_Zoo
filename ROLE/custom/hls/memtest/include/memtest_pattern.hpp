
#ifndef _ROLE_MEMTEST_PATTERN_H_
#define _ROLE_MEMTEST_PATTERN_H_

#include <stdio.h>
#include <iostream>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>
#include "../../../../../HOST/custom/memtest/languages/cplusplus/include/config.h"//debug level define 

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

/*
*/
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
#if DEBUG_LEVEL == TRACE_ALL
#ifndef __SYNTHESIS__
  printf("DEBUG pCountClockCycles counter value = %s\n", internal_counter.to_string().c_str());
#endif //__SYNTHESIS__
#endif      
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
    #if DEBUG_LEVEL == TRACE_ALL
          printf("rank: %d; size: %d; \n", (int) *pi_rank, (int) *pi_size);
    #endif
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
/********************************************
Functions for generating random sequences
********************************************/
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
    (*outBigWord).range(smallWordDim*(i+1)-1,smallWordDim*i)=nextFibonacciNumber;
    prevFibonacciNumber=currentFibonacciNumber;
    currentFibonacciNumber=nextFibonacciNumber;
    nextFibonacciNumber=genNextFibonacciNumber<ADDR_T,SMALLWORD_T>(currentFibonacciNumber,prevFibonacciNumber);
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
    (*outBigWord).range(smallWordDim*(i+1)-1,smallWordDim*i)=nextNumber;
    prevNumber = currentNumber;
    currentNumber = nextNumber;
    nextNumber = (nextNumber + 1 ) xor i;
  }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////This function represent the main custom logic. it receives data form  //
///// the RX and transmit what needed to the TX
///// CUSTOM LOGIC INSERTION HERE
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
// or stopping once done with the run iterations
    switch(processingFSM)
    {
      case FSM_PROCESSING_WAIT_FOR_META:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG proc FSM, I am in the WAIT_FOR_META state\n");
    #endif
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
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG proc FSM, I am in the PROCESSING_PCKT_PROC state\n");
    #endif
//parse the received data
      if ( !sRxpToProcp_Data.empty() && !sProcpToTxp_Data.full())
      {
        netWord = sRxpToProcp_Data.read();
        switch (netWord.tdata.range(MEMTEST_COMMANDS_HIGH_BIT,MEMTEST_COMMANDS_LOW_BIT))
        {
        case TEST_INVLD_CMD:
          /* FWD an echo of the invalid*/
    #if DEBUG_LEVEL == TRACE_ALL
          printf("DEBUG processing the packet with invalid cmd\n");
    #endif
          sProcpToTxp_Data.write(netWord);
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;
        case TEST_STOP_CMD:
          /* call with stop (never started), unset, fwd the stop */
    #if DEBUG_LEVEL == TRACE_ALL
         printf("DEBUG processing the packet with stop cmd\n");
    #endif
          outNetWord.tdata=TEST_STOP_CMD;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 1;
          sProcpToTxp_Data.write(outNetWord);
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;    
        default:
          /* Execute the test if not invalid or stop*/
    #if DEBUG_LEVEL == TRACE_ALL
          printf("DEBUG processing the packet with the address within cmd\n");
    #endif
          max_address_under_test = netWord.tdata(MEMTEST_ADDRESS_HIGH_BIT,MEMTEST_ADDRESS_LOW_BIT);
          max_iterations = netWord.tdata.range(MEMTEST_ITERATIONS_HIGH_BIT,MEMTEST_ITERATIONS_LOW_BIT);
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
         printf("DEBUG processing the packet with the address %s within cmd %s\n", max_address_under_test.to_string().c_str(), max_iterations.to_string().c_str());
    #endif //__SYNTHESIS__
    #endif
          processingFSM = FSM_PROCESSING_START;
          break;
        }
      }
      break;

//The hw can begin to do something
      case FSM_PROCESSING_START:
    #if DEBUG_LEVEL == TRACE_ALL
        printf("DEBUG proc FSM, I am in the START state\n");
    #endif
    //setting everything ready to start
        curr_address_under_test = 0; 
        first_faulty_address = 0; 
        faulty_addresses_cntr = 0;
        local_mem_addr_non_byteaddressable = 0;
        readingCounter=0;
        writingCounter=0;
        testCounter = 0;
        processingFSM = FSM_PROCESSING_WRITE;
        break;

  //run continuously, hence more than once
      case FSM_PROCESSING_CONTINUOUS_RUN:
        testCounter += 1;
        curr_address_under_test = 0; 
        local_mem_addr_non_byteaddressable = 0;
        readingCounter=0;
        writingCounter=0;
        faulty_addresses_cntr = 0;

        //stopping conditions: either a Stop or the maximum iterations
        if(*start_stop && testCounter < max_iterations){
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
          printf("DEBUG processing continuous run (still run, iter %s) max iters: %s\n",testCounter.to_string().c_str(),max_iterations.to_string().c_str());
    #endif //__SYNTHESIS__
    #endif
        // check if need another meta to send out!
        // if already over the MTU size, or with a command (stop case) or with another iteration I need to send out another meta
          if(bytes_sent_for_tx >= PACK_SIZE){
            sProctoTx_Meta.write(outNetMeta);
    #if DEBUG_LEVEL == TRACE_ALL
        std::cout <<  "DEBUG: writing an additional meta with bytes sent equal to " << bytes_sent_for_tx << std::endl;
    #endif
            bytes_sent_for_tx = 0;
          }
        
        processingFSM = FSM_PROCESSING_WRITE;

          break;
        }else{
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
          printf("DEBUG processing continuous run (stop the run at iter %s) max iters: %s \n",testCounter.to_string().c_str(),max_iterations.to_string().c_str());
    #endif //__SYNTHESIS__
    #endif
          //signal the end of the packet with the iteration of the tests performed
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
//Begin to process
      case FSM_PROCESSING_WRITE:
    #if DEBUG_LEVEL == TRACE_ALL
     printf("DEBUG proc FSM, I am in the WRITE state\n");
    #ifndef __SYNTHESIS__
      std::cout << "DEBUG I have to test " << max_address_under_test<< std::endl;
     std::cout << "DEBUG current addr " << local_mem_addr_non_byteaddressable<< std::endl;
    #endif //__SYNTHESIS__
    #endif

    //if not written all the needed memory cells write
      if (curr_address_under_test < max_address_under_test)// max is byte addressable!
      {
    #if DEBUG_LEVEL == TRACE_ALL
        printf("DEBUG WRITE FSM, writing the memory with counter %d\n",writingCounter);
    #endif

    //custom function for sequence generation and writing to the trgt memory
        genXoredSequentialNumbers<local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32, local_mem_word_t, ap_uint<32>,32>(local_mem_addr_non_byteaddressable, local_under_test_memory+local_mem_addr_non_byteaddressable);
       
       
       //emulating on purpose fault injection
        #ifdef FAULT_INJECTION
        //TODO:  place for control fault injection with a function?
        if(testCounter >= 2 && local_mem_addr_non_byteaddressable > 0){
            local_under_test_memory[local_mem_addr_non_byteaddressable] &= static_cast<local_mem_word_t>(0);
        }
        #endif // FAULT_INJECTION

        local_mem_addr_non_byteaddressable += 1;
        curr_address_under_test += LOCAL_MEM_ADDR_OFFSET;
        writingCounter += 1;
        //cc count
        if(!sOutEnableCCIncrement.full()){
          sOutEnableCCIncrement.write(true);
        }
        processingFSM = FSM_PROCESSING_WRITE;
        break;

      } else{
    #if DEBUG_LEVEL == TRACE_ALL
        printf("DEBUG WRITE FSM, done with the write\n");
    #endif
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
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG proc FSM, I am in the READ state\n");
    #endif
      //if not read all the needed memory cells read
        if (!sInClockCounter.empty())
        {
          writing_cntr = sInClockCounter.read();
        }
      
        if (curr_address_under_test < max_address_under_test) 
        // max is byte addressable!
        {
    #if DEBUG_LEVEL == TRACE_ALL
         printf("DEBUG READ FSM, reading the memory\n");
    #endif
    //gold generation and comparison with current memory content
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
          readingCounter += 1;
          if(!sOutEnableCCIncrement.full()){
            sOutEnableCCIncrement.write(true);
          }
          processingFSM = FSM_PROCESSING_READ;
          break;
        } else{ //done with the reads
    #if DEBUG_LEVEL == TRACE_ALL
          printf("DEBUG READ FSM, done with the read\n");
    #endif
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
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing the output of a run\n");
    #endif

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
      }
      break;

      case FSM_PROCESSING_OUTPUT_2:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing the output of a run part 2\n");
    #endif
      
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
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing the output of a run part 3\n");
    #endif
      
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
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing the output of a run part 4\n");
    #endif
      
      if(!sProcpToTxp_Data.full()){
          outNetWord.tdata.range(64-1,64-32) = writing_cntr;
          outNetWord.tdata.range(32-1,0) = reading_cntr;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          sProcpToTxp_Data.write(outNetWord);
          bytes_sent_for_tx += 8;
          processingFSM = FSM_PROCESSING_CONTINUOUS_RUN;
      }
      break;
    }
};

#endif //_ROLE_MEMTEST_PATTERN_H_
