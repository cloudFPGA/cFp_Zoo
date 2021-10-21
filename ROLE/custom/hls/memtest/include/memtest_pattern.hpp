  
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
#define FSM_PROCESSING_DATAFLOW_WRITE 12
#define FSM_PROCESSING_DATAFLOW_READ 13

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


////TODO: parametrize the width?
template<typename Tin, typename Tout, unsigned int arraysize>
void pMyMemtestMemCpy(Tin* in, Tout* out){
#pragma HLS INLINE
  for (unsigned int i = 0; i < arraysize; i++)
  {
#pragma HLS PIPELINE II=1
    *out = *in;
  }
  
}

template<typename T>
void pPingPongBufferingSingleData(T* in, T* out, bool end_of_transmission){
#pragma HLS INLINE

  static bool first_fifo_writing = true;
  static bool init = true;
  static T ping_pong_buffer [2];
#pragma HLS reset variable=first_fifo_writing
#pragma HLS reset variable=init
#pragma HLS reset variable=ping_pong_buffer

//reset emulation
  if(end_of_transmission){
    init = true;
    first_fifo_writing=true;
  }

//buffering
  if(first_fifo_writing){
    ping_pong_buffer[0]=*in;
    if(!init){
      *out = ping_pong_buffer[1];
      init = false;
    }
    first_fifo_writing = false;
  }else{
    *out = ping_pong_buffer[0];
    ping_pong_buffer[1]=*in;
  }

}
const unsigned long long int  max_counter_cc = 18446744070000000000;
//from Xilinx Vitis Accel examples, tempalte from DCO
//https://github.com/Xilinx/Vitis_Accel_Examples/blob/master/cpp_kernels/axi_burst_performance/src/test_kernel_common.hpp
template<typename Tin, typename Tout, unsigned int counter_precision=64>
void perfCounterProc2Mem(hls::stream<Tin>& cmd, Tout * out, int direction, int burst_length, int nmbr_outstanding) {
  
    Tin input_cmd;
    // wait to receive a value to start counting
    ap_uint<counter_precision> cnt = cmd.read();
// keep counting until a value is available
count:
    while (cmd.read_nb(input_cmd) == false) {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_counter_cc
        cnt++;

#if DEBUG_LEVEL == TRACE_ALL
#ifndef __SYNTHESIS__
  printf("DEBUG perfCounterProc counter value = %s\n", cnt.to_string().c_str());
#endif //__SYNTHESIS__
#endif     
    }
    *out =cnt;
}

//from Xilinx Vitis Accel examples, tempalte from DCO
//https://github.com/Xilinx/Vitis_Accel_Examples/blob/master/cpp_kernels/axi_burst_performance/src/test_kernel_common.hpp
template<typename Tin, typename Tout, unsigned int counter_precision=64>
void perfCounterProc(hls::stream<Tin>& cmd, hls::stream<Tout>& out, int direction, int burst_length, int nmbr_outstanding)
{
#pragma HLS INLINE off

    Tin input_cmd;
    // wait to receive a value to start counting
    ap_uint<counter_precision> cnt = cmd.read();
// keep counting until a value is available
count:
    while (cmd.read_nb(input_cmd) == false) {
        cnt++;
        #if DEBUG_LEVEL == TRACE_ALL
#ifndef __SYNTHESIS__
  printf("DEBUG perfCounterProc counter value = %s\n", cnt.to_string().c_str());
#endif //__SYNTHESIS__
#endif     
    }

    // // write out kernel statistics to global memory
     Tout tmp[1];//was 4
     tmp[0] = cnt;
    // tmp[1] = input_cmd;
     //tmp[1] = burst_length;
    // tmp[3] = nmbr_outstanding;
    //memcpy(out, tmp, 4 * sizeof(Tout));
    out.write(tmp[0]);
    //out.write(tmp[1]);
    //out.write(nmbr_outstanding); this
    //out.write(input_cmd); Xilinx use this to count the errors but we are already counting so...
}

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


template<typename ADDR_T, unsigned int sequenceDim, typename BIGWORD_T, typename SMALLWORD_T, unsigned int smallWordDim>
void genXoredSequentialNumbersSecondVersion(ADDR_T curr, BIGWORD_T * outBigWord){
#pragma HLS INLINE 
// REQUIREMENT: must be a multiple
  //assert( (BIGWORD_T%SMALLWORD_T)==0);
  SMALLWORD_T currentNumber = static_cast<SMALLWORD_T>(curr);
  SMALLWORD_T nextNumber = (currentNumber+1) xor 1;
  SMALLWORD_T prevNumber = currentNumber;

  gen_sequence_loop: for (unsigned int i = 0; i < sequenceDim; i++)
  {
#pragma HLS UNROLL
    (*outBigWord).range(smallWordDim*(i+1)-1,smallWordDim*i)=nextNumber;
    prevNumber = currentNumber;
    currentNumber = nextNumber;
    nextNumber = (nextNumber + 1 ) xor i;
  }
}


template<typename ADDR_T, const unsigned int sequenceDim, typename BIGWORD_T, typename SMALLWORD_T, unsigned int smallWordDim>
void genSequentialNumbers(ADDR_T curr, BIGWORD_T * outBigWord){
#pragma HLS INLINE 
  SMALLWORD_T currentNumber = static_cast<SMALLWORD_T>(curr);
  gen_sequence_loop: for (unsigned int i = 0; i < sequenceDim; i++)
  {
#pragma HLS UNROLL factor=sequenceDim
    (*outBigWord).range(smallWordDim*(i+1)-1,smallWordDim*i)=i;
  }
}


template<const unsigned int max_iterations=4000000, const unsigned int buff_dim = 16>
void pWRGenerateData2StreamWrite(
  hls::stream<local_mem_word_t>& generatedData,
  ap_uint<32> * testCounter,
  local_mem_addr_t max_addr_ut)
{
#pragma HLS INLINE off
    static local_mem_word_t tmp_out [buff_dim];
#pragma HLS array_partition variable=tmp_out cyclic factor=2 dim=1

    local_mem_addr_non_byteaddressable_t maddr_non_byte=0;
    local_mem_addr_t curr_address_ut;


  generate_loop:
  for (curr_address_ut = 0; curr_address_ut < max_addr_ut; curr_address_ut+=LOCAL_MEM_ADDR_OFFSET)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    
    genXoredSequentialNumbersSecondVersion
    <local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32, local_mem_word_t,
     ap_uint<32>,32>(maddr_non_byte, 
      tmp_out+(maddr_non_byte%buff_dim));

    local_mem_word_t tmp_out_scalar;
    #ifdef FAULT_INJECTION
         //TODO:  place for control fault injection with a function?
         if(*testCounter >= 2 && maddr_non_byte > 0){
            // tmp_out = local_under_test_memory[local_mem_addr_non_byteaddressable] & static_cast<local_mem_word_t>(0);
            tmp_out_scalar = tmp_out[maddr_non_byte%buff_dim] & static_cast<local_mem_word_t>(0);
          }else{
             tmp_out_scalar = tmp_out[maddr_non_byte%buff_dim];
          }
    #else // FAULT_INJECTION
      tmp_out_scalar = tmp_out[maddr_non_byte%buff_dim];
    #endif // FAULT_INJECTION

    generatedData.write(tmp_out_scalar);
    maddr_non_byte++;
  }
  

}


template <typename Tcntr, const unsigned int max_iterations=4000000,
const unsigned int buff_dim=64*2>
void pWRReadStream2WriteMainMemory(
  hls::stream<Tcntr>& cmd,
hls::stream<local_mem_word_t>& generatedData,
membus_t * lcl_mem,
local_mem_addr_t max_addr_ut,
unsigned int burst_size)
{
#pragma HLS INLINE off
    local_mem_addr_t curr_address_ut;
    static local_mem_word_t tmp_out[buff_dim];
    static local_mem_addr_t end_distance=0;
    static bool last_iteration = false;
#pragma HLS array_partition variable=tmp_out block factor=2 dim=1

    cmd.write(0);
  read_and_write:
  for (curr_address_ut = 0; curr_address_ut < max_addr_ut; curr_address_ut+=LOCAL_MEM_ADDR_OFFSET)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
   //  #ifndef __SYNTHESIS__
   // checking_loop: for (int i = 0; i < LOCAL_MEM_ADDR_OFFSET; i++)
   //        {
   //          std::cout << tmp_out[i].range((i+1)*8-1,i*8) << " ";
   //        }
   //        std::cout << std::endl;
   //  #endif
    tmp_out[curr_address_ut%buff_dim] = generatedData.read();
    //if stored enough data to begin the bursting OR this is last iteration
    end_distance = max_addr_ut-curr_address_ut;
    last_iteration = LOCAL_MEM_ADDR_OFFSET>=curr_address_ut;
    
    if (curr_address_ut%buff_dim==(burst_size-1)  || last_iteration)

   // std::cout << "Burst filled or last iteration, end_distance= " << end_distance << std::endl;
   // std::cout << "test addr " << curr_address_ut%buff_dim << std::endl;
    {
      if (last_iteration)
      {
        memcpy(lcl_mem+curr_address_ut, tmp_out+curr_address_ut%buff_dim, sizeof(local_mem_word_t)*(end_distance));
      }else{
        memcpy(lcl_mem+curr_address_ut, tmp_out+curr_address_ut%buff_dim, sizeof(local_mem_word_t)*burst_size);
      }
    }
   // memcpy(lcl_mem+curr_address_ut, tmp_out+curr_address_ut%buff_dim, sizeof(local_mem_word_t));
  }
    cmd.write(0);
}


template <typename Tcntr, const unsigned int max_iterations=4000000,
const unsigned int buffer_dim = 16>
void pRDMainMemoryRead2StreamData(
  hls::stream<Tcntr>& cmd,
  hls::stream<local_mem_word_t>& readData,
  membus_t * lcl_mem,
  local_mem_addr_t max_addr_ut)
{
#pragma HLS INLINE off

    local_mem_addr_t curr_address_ut;
    static local_mem_word_t tmp_out[buffer_dim];
    const unsigned int partit_factor = 4;
  #pragma HLS array_partition variable=tmp_out cyclic factor=partit_factor dim=1

    cmd.write(0);
  read_data_from_main_mem:
  for (int i = 0, curr_address_ut = 0; curr_address_ut < max_addr_ut; curr_address_ut+=LOCAL_MEM_ADDR_OFFSET, i++)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    memcpy(tmp_out+i%partit_factor, lcl_mem+curr_address_ut, sizeof(local_mem_word_t));
    readData.write(tmp_out[i%partit_factor]);
  }
    cmd.write(0);

}

template<const unsigned int max_iterations=4000000>
void pRDReadDataStreamAndProduceGold(
  hls::stream<local_mem_word_t>& readData,
  local_mem_addr_t max_addr_ut,
  hls::stream<local_mem_word_t>& outReadData,
  hls::stream<local_mem_word_t>& outGoldData)
{

    local_mem_addr_t curr_address_ut;
    local_mem_word_t testingVector;
    local_mem_word_t goldenVector;
    local_mem_addr_non_byteaddressable_t local_mem_addr_non_byteaddressable=0;

  generate_loop:
  for (curr_address_ut = 0; curr_address_ut < max_addr_ut; curr_address_ut+=LOCAL_MEM_ADDR_OFFSET)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    testingVector = readData.read(); 

    genXoredSequentialNumbersSecondVersion<local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32,
     local_mem_word_t, ap_uint<32>,32>(local_mem_addr_non_byteaddressable, &goldenVector);

    outReadData.write(testingVector);
    outGoldData.write(goldenVector);
    local_mem_addr_non_byteaddressable++;
  }
  
}


template<const unsigned int max_iterations=4000000, const unsigned int unrolling_factor= LOCAL_MEM_ADDR_OFFSET, //inability of using define in pragmas solved in 2021.1
 const unsigned int buff_dim=16>
void pRDCompareDataStreams(
  local_mem_addr_t max_addr_ut,
  hls::stream<local_mem_word_t>& sInReadData,
  hls::stream<local_mem_word_t>& sInGoldData,
  hls::stream<ap_uint<64>>& sInCmpRes,
 // hls::stream<local_mem_addr_t>& sInFaultyAddresses, 
  local_mem_addr_t * first_faulty_address)
{
//#pragma HLS INLINE off
    const unsigned int dble_wrd_dim =  LOCAL_MEM_ADDR_OFFSET * 2;
    const unsigned int support_dim  =  LOCAL_MEM_ADDR_OFFSET  * 2;


    local_mem_addr_t curr_address_ut;
    static local_mem_word_t testingVector[buff_dim];
    static local_mem_word_t goldenVector[buff_dim];
    local_mem_addr_non_byteaddressable_t maddr_non_byte=0;


    static ap_uint<8> testingVector_bytes [support_dim];
    static ap_uint<8> goldenVector_bytes [support_dim];
    static bool cmp_ok [support_dim];
    //static ap_uint<32> faulty_addresses_cntr_support_array [support_dim];

//#pragma HLS array_partition variable=faulty_addresses_cntr_support_array cyclic factor=2 dim=1
#pragma HLS array_partition variable=testingVector_bytes cyclic factor=2 dim=1
#pragma HLS array_partition variable=goldenVector_bytes cyclic factor=2 dim=1
#pragma HLS array_partition variable=cmp_ok cyclic factor=2 dim=1



    static local_mem_addr_t first_faulty_address_local=0;
    static local_mem_addr_t faulty_addresses_support_array [support_dim];

   // static ap_uint<32> faulty_addresses_cntr_local=0;
    static bool first_fault_found  = false;
    static ap_uint<LOCAL_MEM_ADDR_OFFSET> faults_founds[2];
    ap_uint<LOCAL_MEM_ADDR_OFFSET> iteration_faults_founds;




    ap_uint<1> k;

      reading_loop:
  for (curr_address_ut = 0, k=0; curr_address_ut < max_addr_ut; curr_address_ut+=LOCAL_MEM_ADDR_OFFSET, k++)
  {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
  testingVector[maddr_non_byte%buff_dim] = sInReadData.read(); 
  goldenVector[maddr_non_byte%buff_dim] = sInGoldData.read(); 

      ap_uint<64> tmpOut = 0; 

   //#pragma HLS dependence variable=faulty_addresses_cntr_support_array inter RAW false
      golden_comparison: for (int i = 0; i < LOCAL_MEM_ADDR_OFFSET; i++)
   {
//#pragma HLS UNROLL factor=unrolling_factor skip_exit_check
        //int idx = (i + curr_address_ut )% support_dim;
        int idx = (i + k*LOCAL_MEM_ADDR_OFFSET);
        testingVector_bytes[idx]=testingVector[maddr_non_byte%buff_dim].range((i+1)*8-1,i*8);
        goldenVector_bytes[idx]=goldenVector[maddr_non_byte%buff_dim].range((i+1)*8-1,i*8);

      cmp_ok[idx] = testingVector_bytes[idx] == goldenVector_bytes[idx];
      tmpOut[i] = !cmp_ok[idx];

      // if (!cmp_ok[idx]) //fault
      // {
        if (!cmp_ok[idx] && !first_fault_found)
        {
          *first_faulty_address=i+curr_address_ut;
          first_fault_found = true;
        }else{
          first_fault_found = first_fault_found;
        }
//        sInFaultyAddresses.write(faulty_addresses_support_array[idx]);
      // }
   }

   // for (int i = 0; i < support_dim; i++)
   // {
   //   faulty_addresses_support_array[i]=0;
   // }

   sInCmpRes.write(tmpOut);
    maddr_non_byte++;
  }

  first_fault_found=false;

}


template<const unsigned int max_iterations=4000000, const unsigned int buff_dim=16,
const unsigned int magic_reduction = 32>
void pRDExtractComparisonResults(
  local_mem_addr_t max_addr_ut,
  hls::stream<ap_uint<64>>& sInCmpRes,
  ap_uint<32> * faulty_addresses_cntr)
{

  local_mem_addr_t curr_address_ut;
  static ap_uint<32> popcount_array  [buff_dim];
  static ap_uint<32> reduction_variable = 0;
  static ap_uint <3> tmp_sum[magic_reduction];//64/2
  #pragma HLS array_partition variable=tmp_sum complete

  popcount_loop:
  for (int i = 0, curr_address_ut = 0; curr_address_ut < max_addr_ut; curr_address_ut+=LOCAL_MEM_ADDR_OFFSET, i = (i+1)%buff_dim)
  {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
      ap_uint<64> tmp_in = sInCmpRes.read();
   // std::cout << " reading  " <<  std::bitset<64>(tmp_in.to_uint64()) << std::endl; 

      for (int j = 0, k=0; j < magic_reduction*2; j+=2, k++)
      {
#pragma HLS UNROLL factor=magic_reduction skip_exit_check
        tmp_sum[k] = tmp_in[j] + tmp_in[j+1];
        if (j>0)
        {
          popcount_array[i] += tmp_sum[k-1];
        }else{
          popcount_array[i]=0;
        }
      }
  }  
  for (int i = 0; i < buff_dim; ++i)
  {
  #pragma HLS PIPELINE
    reduction_variable +=  popcount_array[i] ;
  }

   *faulty_addresses_cntr=reduction_variable;
   reduction_variable=0;
}


void pWriteDataflowMemTest(
  membus_t * lcl_mem0,
  local_mem_addr_t max_address_under_test,
  ap_uint<64> * writing_cntr,
  ap_uint<32> * testCounter)
{
  //#pragma HLS INLINE
     static hls::stream<ap_uint<64>> sWritePrfCntr_cmd("sWritePrfCntr_cmd"); 
     #pragma HLS STREAM variable=sWritePrfCntr_cmd depth=2 dim=1
     static hls::stream<local_mem_word_t> generatedWriteData("generatedWriteData"); 
     #pragma HLS STREAM variable=generatedWriteData depth=2 dim=1
    #pragma HLS DATAFLOW disable_start_propagation
          //Step 1: Generate the data
          pWRGenerateData2StreamWrite<4000000>(generatedWriteData,testCounter,max_address_under_test);
          //Step 2: write 
          pWRReadStream2WriteMainMemory<ap_uint<64>,4000000>(sWritePrfCntr_cmd, generatedWriteData, lcl_mem0, max_address_under_test, 16);
          //Step 2.b: count 
          perfCounterProc2Mem<ap_uint<64>,ap_uint<64>,64>(sWritePrfCntr_cmd, writing_cntr, 0, 256,  16);
      

}


void pReadDataflowMemTest(
  membus_t * lcl_mem1,
  local_mem_addr_t max_address_under_test,
  ap_uint<64> * reading_cntr,
  ap_uint<32> * faulty_addresses_cntr,
  local_mem_addr_t * first_faulty_address)
  {
 static hls::stream<ap_uint<64>> sReadPrfCntr_cmd("sReadPrfCntr_cmd"); 
 #pragma HLS STREAM variable=sReadPrfCntr_cmd depth=2 dim=1
 static hls::stream<local_mem_word_t> generatedReadData("generatedReadData"); 
 #pragma HLS STREAM variable=generatedReadData depth=2 dim=1
  static hls::stream<local_mem_word_t> sReadData("sReadData"); 
 #pragma HLS STREAM variable=sReadData depth=2 dim=1
  static hls::stream<local_mem_word_t> sGoldData("sGoldData"); 
 #pragma HLS STREAM variable=sGoldData depth=2 dim=1

  static hls::stream<ap_uint<64>> sComparisonData("sComparisonData"); 
 #pragma HLS STREAM variable=sComparisonData depth=2 dim=1
 //  static hls::stream<local_mem_addr_t> sFaultyAddresses("sFaultyAddresses"); 
 // #pragma HLS STREAM variable=sFaultyAddresses depth=64 dim=1

#pragma HLS DATAFLOW
      //Step 1: Generate the data
      pRDMainMemoryRead2StreamData<ap_uint<64>,4000000>( sReadPrfCntr_cmd, generatedReadData, lcl_mem1, max_address_under_test);
      //Step 2: write 
      pRDReadDataStreamAndProduceGold<4000000>(generatedReadData, max_address_under_test, sReadData, sGoldData);
      //Step 2.b: count 
      perfCounterProc2Mem<ap_uint<64>,ap_uint<64>,64>(sReadPrfCntr_cmd, reading_cntr, 0, 256,  16);
      //Step 3: compare
      pRDCompareDataStreams<4000000>(max_address_under_test,sReadData, sGoldData, sComparisonData, first_faulty_address);
      //Step 4.a: extract the first faulty address and consume the others
      //Step 4.b: extract the comparison numbers
      pRDExtractComparisonResults<4000000>(max_address_under_test, sComparisonData, faulty_addresses_cntr);
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////This function represent the main custom logic. it receives data form ///
///// the RX and transmit what needed to the TX //////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///////////////////// CUSTOM LOGIC INSERTION HERE ////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
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
  bool *                                           start_stop//,
  // hls::stream<Tevent>                              &sOutEnableCCIncrement,
  // hls::stream<Tevent>                              &sOutResetCounter, 
  // hls::stream<Tevent>                              &sOutGetTheCounter,
  // hls::stream<ap_uint<counter_width>>              &sInClockCounter,
  // hls::stream<ap_uint<counter_width>>              &sOCMDPerfCounter,
  // hls::stream<ap_uint<counter_width>>              &sIResPerfCounter
  #ifdef ENABLE_DDR
                                                                    ,
  //------------------------------------------------------
  //-- SHELL / Role / Mem / Mp1 Interface
  //------------------------------------------------------    
  membus_t                    *lcl_mem0,
  membus_t                    *lcl_mem1
  #endif
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
//#pragma HLS reset variable=testingVector

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
      // sOCMDPerfCounter.write(0);//init the performance counter to not stuck everything at the beginning
      // sOCMDPerfCounter.write(0);//stop the performance counter to not stuck everything at the beginning
      // sIResPerfCounter.read();//pop
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
      // sOCMDPerfCounter.write(0);//init the performance counter to not stuck everything at the beginning
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
      // sOCMDPerfCounter.write(0);//stop the performance counter to not stuck everything at the beginning
      // sIResPerfCounter.read();//pop
          processingFSM = FSM_PROCESSING_START;
          break;
        }
      }
      break;

//The hw can begin to do something
      case FSM_PROCESSING_START:
      // sOCMDPerfCounter.write(0);//init the counter
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
        // sOCMDPerfCounter.write(0);//stop the counter before beginnign the real test
        // sIResPerfCounter.read();//pop the useless value
        processingFSM = FSM_PROCESSING_DATAFLOW_WRITE;//FSM_PROCESSING_WRITE;
        break;

  //run continuously, hence more than once
      case FSM_PROCESSING_CONTINUOUS_RUN:
      // sOCMDPerfCounter.write(0);//init the counter
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
        // sOCMDPerfCounter.write(0);//stop the counter before beginnign the real test
        // sIResPerfCounter.read();//pop the two useless values
        processingFSM = FSM_PROCESSING_DATAFLOW_WRITE;//FSM_PROCESSING_WRITE;

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
        // sOCMDPerfCounter.write(0);//stop the counter before beginnign the real test
        // sIResPerfCounter.read();//pop the two useless values
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;
        }

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////Custom User Processing Logic here///////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//Begin to process
    //   case FSM_PROCESSING_WRITE:
    // #if DEBUG_LEVEL == TRACE_ALL
    //  printf("DEBUG proc FSM, I am in the WRITE state\n");
    // #ifndef __SYNTHESIS__
    //   std::cout << "DEBUG I have to test " << max_address_under_test<< std::endl;
    //  std::cout << "DEBUG current addr " << local_mem_addr_non_byteaddressable<< std::endl;
    // #endif //__SYNTHESIS__
    // #endif
    // // sOCMDPerfCounter.write(0);//start

    // //if not written all the needed memory cells write
    //   if (curr_address_under_test < max_address_under_test)// max is byte addressable!
    //   {
    // #if DEBUG_LEVEL == TRACE_ALL
    //     printf("DEBUG WRITE FSM, writing the memory with counter %d\n",writingCounter);
    // #endif
    // // //custom function for sequence generation and writing to the trgt memory
    //      genXoredSequentialNumbers<local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32, local_mem_word_t, ap_uint<32>,32>(local_mem_addr_non_byteaddressable, local_under_test_memory+local_mem_addr_non_byteaddressable);
       
    //     //emulating on purpose fault injection
    //      #ifdef FAULT_INJECTION
    //      //TODO:  place for control fault injection with a function?
    //      if(testCounter >= 2 && local_mem_addr_non_byteaddressable > 0){
    //          local_under_test_memory[local_mem_addr_non_byteaddressable] &= static_cast<local_mem_word_t>(0);
    //      }
    //      #endif // FAULT_INJECTION
    //     local_mem_addr_t tmpLclAddress = curr_address_under_test + LOCAL_MEM_ADDR_OFFSET;
    //     bool last_write = (tmpLclAddress >= max_address_under_test);

    //     //pPingPongBufferingSingleData<membus_t>(local_under_test_memory+local_mem_addr_non_byteaddressable, lcl_mem0+curr_address_under_test,last_write);
    //     memcpy(lcl_mem0+curr_address_under_test,local_under_test_memory+local_mem_addr_non_byteaddressable,sizeof(local_mem_word_t));
    // #if DEBUG_LEVEL == TRACE_ALL
    // #ifndef __SYNTHESIS__
    //     std::cout << "Writing: " << *(local_under_test_memory+local_mem_addr_non_byteaddressable) << std::endl;
    //     std::cout << " and  mem: " << *(lcl_mem0+curr_address_under_test) << std::endl;
    // #endif
    // #endif
    //     // sOCMDPerfCounter.write(0);//stop the counter before beginnign the real test
    //     //writing_cntr += sIResPerfCounter.read();//pop the two useless values
        
    //     //cc count
    //     // if(!sOutEnableCCIncrement.full()){
    //     //   sOutEnableCCIncrement.write(true);
    //     // }
    //     local_mem_addr_non_byteaddressable += 1;
    //     curr_address_under_test = tmpLclAddress;
    //     writingCounter += 1;

    //     processingFSM = FSM_PROCESSING_WRITE;
    //     break;

    //   } else{
    // #if DEBUG_LEVEL == TRACE_ALL
    //     printf("DEBUG WRITE FSM, done with the write\n");
    // #endif
    //     local_mem_addr_non_byteaddressable = 0;
    //     curr_address_under_test = 0;
    //     // if(!sOutGetTheCounter.full()){
    //     //   sOutGetTheCounter.write(true);
    //     // }
    //     // if(!sOutResetCounter.full()){
    //     //   sOutResetCounter.write(true);
    //     // }
    //     // sOCMDPerfCounter.write(0);//stop the counter before beginnign the real test
    //     // sIResPerfCounter.read();//pop the two useless values
    //     processingFSM = FSM_PROCESSING_READ;
    //     break;
    //   }

    // case FSM_PROCESSING_READ:
    // #if DEBUG_LEVEL == TRACE_ALL
    //   printf("DEBUG proc FSM, I am in the READ state\n");
    // #endif
    //   // sOCMDPerfCounter.write(0);

    //   //if not read all the needed memory cells read
    //     // if (!sInClockCounter.empty())
    //     // {
    //     //   writing_cntr = sInClockCounter.read();
    //     //   //sInClockCounter.read();
    //     // }
      
    //     if (curr_address_under_test < max_address_under_test) 
    //     // max is byte addressable!
    //     {
    // #if DEBUG_LEVEL == TRACE_ALL
    //      printf("DEBUG READ FSM, reading the memory\n");
    // #endif

    // //gold generation and comparison with current memory content
    //       local_mem_addr_t tmpLclRDAddress = curr_address_under_test + LOCAL_MEM_ADDR_OFFSET;
    //       bool last_read = (tmpLclRDAddress >= max_address_under_test);
    //       //pPingPongBufferingSingleData<membus_t>(lcl_mem1+curr_address_under_test, local_under_test_memory+local_mem_addr_non_byteaddressable, last_read);
    //      memcpy(local_under_test_memory+local_mem_addr_non_byteaddressable,lcl_mem1+curr_address_under_test,sizeof(local_mem_word_t));

    //       // sOCMDPerfCounter.write(0);//stop the counter before beginnign the real test
    //       //reading_cntr += sIResPerfCounter.read();//pop the two useless values

    //       genXoredSequentialNumbers<local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32, local_mem_word_t, ap_uint<32>,32>(local_mem_addr_non_byteaddressable, &goldenVector);
    //       testingVector=local_under_test_memory[local_mem_addr_non_byteaddressable];

    //       golden_comparison: for (int i = 0; i < LOCAL_MEM_ADDR_OFFSET; ++i)
    //       {
    // #pragma HLS UNROLL
    // #ifndef __SYNTHESIS__
    //         std::cout << "Comparing test: " << testingVector.range((i+1)*8-1,i*8) << " and  gold: " << goldenVector.range((i+1)*8-1,i*8) << std::endl;
    // #endif
    //         if (testingVector.range((i+1)*8-1,i*8) != goldenVector.range((i+1)*8-1,i*8))
    //         {
    //           if (faulty_addresses_cntr == 0) //first fault
    //           {
    //             first_faulty_address = i+curr_address_under_test; //save the fault address
    //           }
    //           faulty_addresses_cntr += 1; //increment the fault counter
    //         } 
    //       }
    //       curr_address_under_test = tmpLclRDAddress; //next test
    //       local_mem_addr_non_byteaddressable += 1;
    //       readingCounter += 1;
    //       // if(!sOutEnableCCIncrement.full()){
    //       //   sOutEnableCCIncrement.write(true);
    //       // }
    //       processingFSM = FSM_PROCESSING_READ;
    //       break;
    //     } else{ //done with the reads
    // #if DEBUG_LEVEL == TRACE_ALL
    //       printf("DEBUG READ FSM, done with the read\n");
    // #endif
    //     // sOCMDPerfCounter.write(0);
    //     // sIResPerfCounter.read();
    //     //   if(!sOutGetTheCounter.full()){
    //     //     sOutGetTheCounter.write(true);
    //     //   }
    //     //   if(!sOutResetCounter.full()){
    //     //     sOutResetCounter.write(true);
    //     //   }
    //       processingFSM = FSM_PROCESSING_OUTPUT;
    //       break;
    //     }


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////Custom output management Logic here/////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
      case FSM_PROCESSING_OUTPUT:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing the output of a run\n");
    #endif
    // sOCMDPerfCounter.write(0);
      // if (!sInClockCounter.empty())
      // {
      //     //sInClockCounter.read();
      //     reading_cntr = sInClockCounter.read();
      // }
      
      if(!sProcpToTxp_Data.full()){
          outNetWord.tdata = max_address_under_test;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          sProcpToTxp_Data.write(outNetWord);
          bytes_sent_for_tx += 8;
          processingFSM = FSM_PROCESSING_OUTPUT_2;
      }
      // sOCMDPerfCounter.write(0);
      // sIResPerfCounter.read();
      break;

      case FSM_PROCESSING_OUTPUT_2:
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
      printf("DEBUG processing the output of a run part 2; faulty address cntr %s\n",faulty_addresses_cntr.to_string().c_str());
    #endif
    #endif
        // sOCMDPerfCounter.write(0);
        // sOCMDPerfCounter.write(0);
        // sIResPerfCounter.read();
      
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
    #ifndef __SYNTHESIS__
      printf("DEBUG processing the output of a run part 3: first faulty address %s\n",first_faulty_address.to_string().c_str());
    #endif
    #endif
        // sOCMDPerfCounter.write(0);
        // sOCMDPerfCounter.write(0);
        // sIResPerfCounter.read();
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
        // sOCMDPerfCounter.write(0);
        // sOCMDPerfCounter.write(0);
        // sIResPerfCounter.read();
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


      //Begin to process
      case FSM_PROCESSING_DATAFLOW_WRITE:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing write dataflow\n");
    #endif
    pWriteDataflowMemTest( lcl_mem0, max_address_under_test, &writing_cntr,&testCounter);
      processingFSM = FSM_PROCESSING_DATAFLOW_READ;
      break;
      
      //Begin to process
      case FSM_PROCESSING_DATAFLOW_READ:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing read dataflow\n");
    #endif
      pReadDataflowMemTest(lcl_mem1,max_address_under_test,&reading_cntr,&faulty_addresses_cntr, &first_faulty_address);
      processingFSM = FSM_PROCESSING_OUTPUT;
      break;


    }
};

#endif //_ROLE_MEMTEST_PATTERN_H_
