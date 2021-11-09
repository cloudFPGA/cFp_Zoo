/*****************************************************************************
 * @file       memtest_pattern_library.hpp
 * @brief      A library for memory test functionalities: sequence generation, reading, writing
 * @author     DCO
 * @date       September 2021
 *----------------------------------------------------------------------------
 *
 * @details      
 *
 * @deprecated   
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup MemtestHLS
 * @addtogroup MemtestHLS
 * \{
 *****************************************************************************/
#ifndef _ROLE_MEMTEST_PATTERN_LIBRARY_HPP_
#define _ROLE_MEMTEST_PATTERN_LIBRARY_HPP_

#include <stdio.h>
#include <iostream>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>
#include "../../../../../HOST/custom/memtest/languages/cplusplus/include/config.h"//debug level define 

#include "network.hpp"

using namespace hls;


#define LOCAL_MEM_WORD_SIZE 512
#define LOCAL_MEM_ADDR_SIZE 40

typedef ap_uint<LOCAL_MEM_WORD_SIZE>  local_mem_word_t;
typedef ap_uint<LOCAL_MEM_ADDR_SIZE>  local_mem_addr_t; 
#define LOCAL_MEM_ADDR_SIZE_NON_BYTE_ADDRESSABLE 40 // TODO: to parametrize better
typedef ap_uint<LOCAL_MEM_ADDR_SIZE_NON_BYTE_ADDRESSABLE>  local_mem_addr_non_byteaddressable_t; 
#define LOCAL_MEM_ADDR_OFFSET (LOCAL_MEM_WORD_SIZE/8) //byte addres offset
#define LOCAL_MEM_WORD_BYTE_SIZE (LOCAL_MEM_WORD_SIZE/8) //byte size of a local mem word

//////////////////////////////////////////////////////////////////////////////
//////////////////Begin of Generate Functions/////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/********************************************
Functions for generating (pseudo)random sequences or for pattern generation
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

template<typename ADDR_T, typename BIGWORD_T>
void genXoredNumbersSingleWord(ADDR_T curr, BIGWORD_T * outBigWord){
#pragma HLS INLINE 
  BIGWORD_T currentNumber = static_cast<BIGWORD_T>(curr);
  BIGWORD_T nextNumber = (currentNumber+1) xor 1;
  *outBigWord=nextNumber;
}


template<typename ADDR_T, typename BIGWORD_T>
void genSequentialNumbers(ADDR_T curr, BIGWORD_T * outBigWord){
#pragma HLS INLINE 
  *outBigWord = curr+1;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////End of Generate Functions///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//////////////////Begin of WriteFunctions/////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * @brief A function that generate a streams of data according to function and writes them
 * on a stream
 * CHECK: FAULT_INJECTION define insert after the third test some faults
 *
 * @param[out]   sOutGeneratedData the output stream for deta generation
 * @param[in]   max_addr_ut the maximum address to test
 * @param[in]   testCounter the current times of re-executing this test
 * 
 * @return Nothing.
 *****************************************************************************/
template<const unsigned int max_iterations=4000000, const unsigned int buff_dim = 16>
void pWRGenerateData2WriteOnStream(
  hls::stream<local_mem_word_t>& sOutGeneratedData,
  ap_uint<32> * testCounter,
  local_mem_addr_t max_addr_ut)
{
#pragma HLS INLINE off
    static local_mem_word_t tmp_out [buff_dim];
#pragma HLS array_partition variable=tmp_out cyclic factor=2 dim=1

    static local_mem_addr_t curr_address_ut=0;
  generate_loop:
  for (; curr_address_ut < max_addr_ut; )
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
//if free to generate or need backpressure
    if (!sOutGeneratedData.full())
    {
      //Step 1) Generated the data according to a function

      //  genXoredSequentialNumbersSecondVersion
      //  <local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32, local_mem_word_t,
      //   ap_uint<32>,32>(maddr_non_byte, 
      //    tmp_out+(maddr_non_byte));
      //genSequentialNumbers<local_mem_addr_non_byteaddressable_t,local_mem_word_t>(curr_address_ut*64, tmp_out+(maddr_non_byte));
      genXoredNumbersSingleWord<local_mem_addr_t,
      local_mem_word_t>(curr_address_ut, tmp_out+(curr_address_ut%buff_dim));

//Step 2) Optional fault injection
      local_mem_word_t tmp_out_scalar;
      #ifdef FAULT_INJECTION
          //TODO:  place for control fault injection with a function?
          if(*testCounter >= 2 && curr_address_ut > 0){
              tmp_out_scalar = tmp_out[curr_address_ut%buff_dim] & static_cast<local_mem_word_t>(0);
            }else{
              tmp_out_scalar = tmp_out[curr_address_ut%buff_dim];
            }
      #else // FAULT_INJECTION
        tmp_out_scalar = tmp_out[curr_address_ut%buff_dim];
      #endif // FAULT_INJECTION
      //Step 3) write out to the next macrostep and going to the next element
      sOutGeneratedData.write(tmp_out_scalar);
      curr_address_ut++;
    }
    //else account for a newer cycle to write
  }
  curr_address_ut=0;
  

}

/*****************************************************************************
 * @brief A function that read a stream of data and write them in a run-time variable burst-size
 *
 * @param[out]  sOutCmd the output stream for cmd generation
 * @param[in]   sInGeneratedData the input stream of generated data
 * @param[out]  lcl_mem the memory mapped ptr
 * @param[in]   max_addr_ut the maximum address to test
 * @param[in]   burst_size the run-time variable burst size to write
 * 
 * @return Nothing.
 *****************************************************************************/
template <typename Tcntr, const unsigned int max_iterations=4000000,
const unsigned int buff_dim=64*2>
void pWRStream2WriteMainMemory(
  hls::stream<Tcntr>& sOutCmd,
hls::stream<local_mem_word_t>& sInGeneratedData,
membus_t * lcl_mem,
local_mem_addr_t max_addr_ut,
unsigned int burst_size)
{
#pragma HLS INLINE off
    local_mem_addr_t curr_address_ut;
    local_mem_addr_t curr_writing_addr;
    static local_mem_word_t tmp_out[buff_dim];
    static unsigned int end_distance=0;
    static bool last_iteration = false;
    static bool activated_cntr = false;
#pragma HLS array_partition variable=tmp_out block factor=2 dim=1

  int idx, written_i;
  int ptrs_difference=0;
  unsigned int last_words=max_addr_ut;
  unsigned int maximum_usable_fifo_words=buff_dim-buff_dim%burst_size;
  read_and_write:
  for (curr_address_ut = 0, idx=0, curr_writing_addr=0, written_i=0; curr_address_ut < max_addr_ut; curr_address_ut++)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    if (!sInGeneratedData.empty())
    {
      tmp_out[idx] = sInGeneratedData.read();
        #if DEBUG_LEVEL == TRACE_ALL
        #ifndef __SYNTHESIS__
        std::cout << tmp_out[idx] << std::endl;
        #endif//synth
        #endif//debug lvl
      //if stored enough data to begin the bursting OR this is last iteration
      end_distance = max_addr_ut-curr_address_ut;
      last_iteration = 1>=end_distance;
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
      std::cout << "test addr " << idx << " current address " << curr_address_ut << " max addr " << max_addr_ut << std::endl;
      std::cout << "ptrs_difference " << ptrs_difference << " last_iteration " << last_iteration <<std::endl;
      #endif
      #endif
//accumulated a burst or last iteration
      if ((ptrs_difference>0 && ptrs_difference>=burst_size-1) || (last_iteration))
      {
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
      std::cout << "Burst filled or last iteration, end distance will be= " << end_distance << std::endl;
      #endif
      #endif
        if (!last_iteration)
        {
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << "BURST transferring " << burst_size << " words at " << curr_writing_addr << " address, from  " << written_i << std::endl;
      #endif
      #endif
          if(!activated_cntr){
            sOutCmd.write(0);
            activated_cntr = true;
          }else{
            sOutCmd.write(1);
          }
          memcpy(lcl_mem+curr_writing_addr, tmp_out+written_i, sizeof(local_mem_word_t)*burst_size);
          sOutCmd.write(1);
          curr_writing_addr+=burst_size;
          written_i= (written_i+burst_size)%maximum_usable_fifo_words;
          ptrs_difference-=burst_size;
          last_words=end_distance;
        }else{
      #if DEBUG_LEVEL == TRACE_ALL
      //#ifndef __SYNTHESIS__
          std::cout << "LAST transferring " << last_words << " words at " << curr_writing_addr << " address, from  " << written_i << std::endl;
      //#endif
      #endif
          if(!activated_cntr){
            sOutCmd.write(0);
            activated_cntr = true;
          }else{
            sOutCmd.write(1);
          }
          memcpy(lcl_mem+curr_writing_addr, tmp_out+written_i, sizeof(local_mem_word_t)*(last_words));
          sOutCmd.write(1);
          curr_writing_addr+=(last_words);
          written_i=(written_i+last_words)%maximum_usable_fifo_words;
          ptrs_difference-=last_words;
        }
      }
      if(idx==maximum_usable_fifo_words-1){
        idx=0;
      }else{
        idx++;
      }
      ptrs_difference++;
    }else{
      curr_address_ut--;
    }
  }
    sOutCmd.write(0);//quit everything
    end_distance=0;
    last_iteration = false;
}

/*****************************************************************************
 * @brief Simple version of a write memtest that write up to a given maximum address
 * No control on the burst size or on the first faulty address
 *
 * @param[out]  sOutCmd the perf counter cmd stream
 * @param[in]   lcl_mem the virtual memory mapped pointer
 * @param[in]   max_addr_ut the maximum address to test
 * @param[in]   burst_size the run-time variable burst size NOT MEANINGFUL HERE
 * 
 * @return Nothing.
 *****************************************************************************/
template <typename Tcntr, const unsigned int max_iterations=4000000,const unsigned int buff_dim=64*2>
void pWriteSimplerTestMemTest(
hls::stream<Tcntr>& sOutCmd,
membus_t * lcl_mem,
local_mem_addr_t max_addr_ut,
unsigned int burst_size)
{
//Step 1) start counting
  sOutCmd.write(0);
  local_mem_addr_t curr_address_ut;
//Step 2) write a simple pattern
  for (curr_address_ut = 0; curr_address_ut < max_addr_ut; curr_address_ut++)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
    printf("Writing address %s with max %s\n",curr_address_ut.to_string().c_str(), max_addr_ut.to_string().c_str());
    #endif
    #endif
    lcl_mem[curr_address_ut]=curr_address_ut+1;
  }
//Step 3) stop counting
  sOutCmd.write(0);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////End of Write Functions//////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//////////////////Begin of Read Functions/////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * @brief Simple version of a read memtest that read up to a given maximum address
 * No control on the burst size or on the first faulty address
 *
 * @param[out]  sOutCmd the perf counter cmd stream
 * @param[in]   lcl_mem the virtual memory mapped pointer
 * @param[in]   max_addr_ut the maximum address to test
 * @param[in]   burst_size the run-time variable burst size NOT MEANINGFUL HERE
 * @param[out]  faulty_addresses_cntr the fault cntr
 * @param[out]  first_faulty_address the fairst faulty address NOT MEANINGFUL HERE
 * 
 * @return Nothing.
 *****************************************************************************/
template <typename Tcntr, const unsigned int max_iterations=4000000,const unsigned int buff_dim=64*2>
void pReadSimplerTestMemTest(
hls::stream<Tcntr>& sOutCmd,
membus_t * lcl_mem,
local_mem_addr_t max_addr_ut,
unsigned int burst_size ,
ap_uint<32> * faulty_addresses_cntr,
local_mem_addr_t * first_faulty_address)
{
  local_mem_addr_t curr_address_ut;
  int faults = 0;
//Step 1) start counting
  sOutCmd.write(0);
  for (curr_address_ut = 0; curr_address_ut < max_addr_ut; curr_address_ut++){
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
  printf("Tst=%s vs gld=%s\n",lcl_mem[curr_address_ut].to_string().c_str(),(curr_address_ut+1).to_string().c_str());
  #endif 
  #endif
//Step 2) read a simple pattern and fault checks
    faults +=  (lcl_mem[curr_address_ut]!=(curr_address_ut+1)) ? 1 : 0;
  }
//Step 3) stop counting
  sOutCmd.write(0);
  *faulty_addresses_cntr = faults;
  *first_faulty_address = 0;
}


/*****************************************************************************
 * @brief Read a variable burst_size amount of data and output on a stream 
 * and count the cc needed just 4 transfer but without using memcpy
 *
 * @param[out]  sOutCmd the perf counter cmd stream
 * @param[out]  sOutReadData the read data stream
 * @param[in]   lcl_mem the virtual memory mapped pointer
 * @param[in]   max_addr_ut the maximum address to test
 * @param[in]   burst_size the run-time variable burst size
 *
 * @return Nothing.
 *****************************************************************************/
template <typename Tcntr, const unsigned int max_iterations=4000000,
const unsigned int buff_dim=64*2>
void pRDRead2StreamDataVariableBurstNoMemCpy(
  hls::stream<Tcntr>& sOutCmd,
  hls::stream<local_mem_word_t>& sOutReadData,
  membus_t * lcl_mem,
  local_mem_addr_t max_addr_ut,
    unsigned int burst_size)
{
#pragma HLS INLINE off
#pragma HLS interface ap_ctrl_none port=return

    local_mem_addr_t curr_address_ut;
    static local_mem_word_t tmp_out[buff_dim];
#pragma HLS array_partition variable=tmp_out cyclic factor=2 dim=1
////TODO: Check if with 64 is better
    static local_mem_addr_t curr_reading_addr;
    static unsigned int end_distance=0;
    static bool transfer_less_than_burst = false;
    static bool activated_cntr = false;
    static bool can_read_data = false;

  read_data_from_main_mem:
  int reading_mm_i = 0;//how much filled the buff
  int consumed_fifo_i = 0;//how much already outputed
  int missing_words=0;
  for (curr_address_ut = 0, curr_reading_addr=0; curr_address_ut < max_addr_ut; )
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    if (!sOutReadData.full())
    {
      end_distance = max_addr_ut-curr_reading_addr;
      transfer_less_than_burst = burst_size>end_distance;
      //i have data to crunch, and eithre the fifo can read a burst or consumed enough to tx less than a burst)
      can_read_data=(end_distance > 0) ;
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << "Max " << max_addr_ut << " reading at " << curr_reading_addr << " reading fifo at " << reading_mm_i <<  " consumed at " << consumed_fifo_i << std::endl;
          std::cout << "End dst " << end_distance << " is last? " << transfer_less_than_burst << std::endl;
          std::cout << "curr_address_ut " << curr_address_ut << std::endl;  
      #endif
      #endif
      //if more than a burst size to available or the last iteration
      if(can_read_data){
        if (!transfer_less_than_burst)
        {
          //read a burst
          if(!activated_cntr){
            activated_cntr = true;
          pReadAxiMemMapped2HlsStreamCountFirst<membus_t,
          local_mem_word_t,64>(lcl_mem+curr_reading_addr, 
          sOutReadData, burst_size, sOutCmd);
          }else{
          pReadAxiMemMapped2HlsStreamCountActivated<membus_t,
          local_mem_word_t,64>(lcl_mem+curr_reading_addr, 
          sOutReadData, burst_size, sOutCmd);
          }
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << "BURST reading " << burst_size << " words from " << curr_reading_addr << " address, to  " << reading_mm_i << std::endl;
      #endif
      #endif
          curr_reading_addr+=burst_size;
          reading_mm_i=(reading_mm_i+burst_size)%buff_dim;
        }else{
          //read the missing words
          missing_words= end_distance%burst_size;
          if(!activated_cntr){
            activated_cntr = true;
          pReadAxiMemMapped2HlsStreamCountFirst<membus_t,
          local_mem_word_t,64>(lcl_mem+curr_reading_addr,
           sOutReadData, missing_words, sOutCmd);
          }else{
          pReadAxiMemMapped2HlsStreamCountActivated<membus_t,
          local_mem_word_t,64>(lcl_mem+curr_reading_addr, 
          sOutReadData, missing_words, sOutCmd);
          }
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << "LAST reading " << missing_words << " words from " << curr_reading_addr << " address, to  " << reading_mm_i << std::endl;
      #endif
      #endif
          curr_reading_addr+=missing_words;
          reading_mm_i=(reading_mm_i+missing_words)%buff_dim;
        }
      }

      //// TODO: add this logic to end before the loop
      //// Example: with burst 64 the loop need less CC to consume everything
      //if() still data to read and to consume
      curr_address_ut++;
      //else{
      //  curr_address_ut=max_addr_ut;
      //}
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
        std::cout <<std::endl;
      #endif
     #endif
    }
  }
  sOutCmd.write(0);
  //reset
  end_distance=0;
  transfer_less_than_burst = false;
  activated_cntr=false;
  can_read_data=false;
}


/*****************************************************************************
 * @brief Read a data stream and produce the gold expected value
 *
 * @param[in]   sInReadData the input read data stream
 * @param[in]   max_addr_ut the maximum address to test
 * @param[in]   sOutReadData the copy of the input data stream
 * @param[in]   sOutGoldData the golden expected value stream
 *
 * @return Nothing.
 *****************************************************************************/
template<const unsigned int max_iterations=4000000>
void pRDReadDataStreamAndProduceGold(
  hls::stream<local_mem_word_t>& sInReadData,
  local_mem_addr_t max_addr_ut,
  hls::stream<local_mem_word_t>& sOutReadData,
  hls::stream<local_mem_word_t>& sOutGoldData)
{
#pragma HLS interface ap_ctrl_none port=return
    static local_mem_addr_t curr_address_ut= 0;
    local_mem_word_t testingVector;
    local_mem_word_t goldenVector;

  generate_loop:
  for (; curr_address_ut < max_addr_ut; )
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    if (!sInReadData.empty() && !sOutReadData.full() &&  !sOutGoldData.full())
    {
      testingVector = sInReadData.read(); 

      // genXoredSequentialNumbersSecondVersion<local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32,
      //  local_mem_word_t, ap_uint<32>,32>(local_mem_addr_non_byteaddressable, &goldenVector);
      genXoredNumbersSingleWord<local_mem_addr_t,
      local_mem_word_t>(curr_address_ut, &goldenVector);
      //genSequentialNumbers<local_mem_addr_non_byteaddressable_t,local_mem_word_t>(curr_address_ut*64, &goldenVector);

      sOutReadData.write(testingVector);
      sOutGoldData.write(goldenVector);
      curr_address_ut++;
    }
  }
  curr_address_ut = 0;
}


/*****************************************************************************
 * @brief Read two streams, compare them and output the number of faults and the first faulty address
 *
 * @param[in]  max_addr_ut the maximum address to test
 * @param[in]  sInReadData the read data stream
 * @param[in]  sInGoldData the gold data stream
 * @param[out] faulty_addresses_cntr the fault cntr ptr
 * @param[out] first_faulty_address the first fault address ptr
 *
 * @return Nothing.
 *****************************************************************************/
template<const unsigned int max_iterations=4000000, const unsigned int buff_dim=16>
void pRDCmpStreamsCntWordAligned(
  local_mem_addr_t max_addr_ut,
  hls::stream<local_mem_word_t>& sInReadData,
  hls::stream<local_mem_word_t>& sInGoldData,
  ap_uint<32> * faulty_addresses_cntr,
  local_mem_addr_t * first_faulty_address)
{
#pragma HLS INLINE off
#pragma HLS interface ap_ctrl_none port=return
    local_mem_addr_t curr_address_ut;
    static local_mem_word_t testingVector[buff_dim];
    static local_mem_word_t goldenVector[buff_dim];
    local_mem_addr_non_byteaddressable_t maddr_non_byte=0;
    static bool cmp_ok [buff_dim];
    static ap_uint<32> faulty_addresses_cntr_support_array [buff_dim];

    static bool first_fault_found  = false;
    static ap_uint<32> faulty_addresses_cntr_local;

      reading_loop:
  for (curr_address_ut = 0, faulty_addresses_cntr_local=0; curr_address_ut < max_addr_ut; curr_address_ut++)
  {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
#pragma HLS dependence variable=faulty_addresses_cntr_support_array inter RAW distance=buff_dim true

    if (!sInReadData.empty() && !sInGoldData.empty())
    {
      testingVector[maddr_non_byte%buff_dim] = sInReadData.read(); 
      goldenVector[maddr_non_byte%buff_dim] = sInGoldData.read(); 

      int idx=maddr_non_byte%buff_dim;
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << maddr_non_byte << " tst=" << testingVector[idx] << " vs gld=" << goldenVector[idx] << std::endl;
      #endif
      #endif
      bool cmp_results = testingVector[idx] == goldenVector[idx];
      cmp_ok[idx]=cmp_results;
      if(!cmp_results ){
        faulty_addresses_cntr_support_array[idx]++;
        if (!first_fault_found)
        {
          *first_faulty_address=curr_address_ut*64;
          first_fault_found = true;
        }else{
          first_fault_found = first_fault_found;
        }
      }

        maddr_non_byte++;
    }else{
      curr_address_ut--;
    }
  }
 // std::cout << std::endl;

  for (int i = 0; i < buff_dim; i++)
  {
#pragma HLS PIPELINE
    faulty_addresses_cntr_local += faulty_addresses_cntr_support_array[i];
    faulty_addresses_cntr_support_array[i]=0;
  }
  
  first_fault_found=false;
*faulty_addresses_cntr=faulty_addresses_cntr_local;

}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////DEPRECATED//////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 * @brief Read two streams, compare them and output the number of faults and 
 * the first faulty address, but check every single byte
 * NOT USED
 * @deprecated
 * @param[in]  max_addr_ut the maximum address to test
 * @param[in]  sInReadData the read data stream
 * @param[in]  sInGoldData the gold data stream
 * @param[out] faulty_addresses_cntr the fault cntr ptr
 * @param[out] first_faulty_address the first fault address ptr
 *
 * @return Nothing.
 *****************************************************************************/
template<const unsigned int max_iterations=4000000, const unsigned int unrolling_factor= LOCAL_MEM_ADDR_OFFSET, //inability of using define in pragmas solved in 2021.1
 const unsigned int buff_dim=16>
void pRDCompareDataStreamsCount(
  local_mem_addr_t max_addr_ut,
  hls::stream<local_mem_word_t>& sInReadData,
  hls::stream<local_mem_word_t>& sInGoldData,
  ap_uint<32> * faulty_addresses_cntr,
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
    static ap_uint<32> faulty_addresses_cntr_support_array [support_dim];

#pragma HLS array_partition variable=faulty_addresses_cntr_support_array cyclic factor=2 dim=1
#pragma HLS array_partition variable=testingVector_bytes cyclic factor=2 dim=1
#pragma HLS array_partition variable=goldenVector_bytes cyclic factor=2 dim=1
#pragma HLS array_partition variable=cmp_ok cyclic factor=2 dim=1

    static bool first_fault_found  = false;
    static ap_uint<32> faulty_addresses_cntr_local;

    ap_uint<1> k;
      reading_loop:
  for (curr_address_ut = 0, k=0, faulty_addresses_cntr_local=0; curr_address_ut < max_addr_ut; k++, curr_address_ut++)
  {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    if (!sInReadData.empty() && !sInGoldData.empty())
    {
      testingVector[maddr_non_byte%buff_dim] = sInReadData.read(); 
      goldenVector[maddr_non_byte%buff_dim] = sInGoldData.read(); 
          golden_comparison: for (int i = 0; i < LOCAL_MEM_ADDR_OFFSET; i++)
       {
    //#pragma HLS UNROLL factor=unrolling_factor skip_exit_check
            int idx = (i + k*LOCAL_MEM_ADDR_OFFSET);
            testingVector_bytes[idx]=testingVector[maddr_non_byte%buff_dim].range((i+1)*8-1,i*8);
            goldenVector_bytes[idx]=goldenVector[maddr_non_byte%buff_dim].range((i+1)*8-1,i*8);
        #if DEBUG_LEVEL == TRACE_ALL
        #ifndef __SYNTHESIS__
            std::cout << " tst=" << testingVector_bytes[idx] << " vs gld=" << goldenVector_bytes[idx] ;
        #endif
        #endif

          cmp_ok[idx] = testingVector_bytes[idx] == goldenVector_bytes[idx];
          if(!cmp_ok[idx] ){
            faulty_addresses_cntr_support_array[i+k]++;
            if (!first_fault_found)
            {
              *first_faulty_address=i+curr_address_ut*64;
              first_fault_found = true;
            }else{
              first_fault_found = first_fault_found;
            }
          }
       }
        #if DEBUG_LEVEL == TRACE_ALL
        #ifndef __SYNTHESIS__
       std::cout << std::endl;
       #endif
       #endif
       //sInCmpRes.write(tmpOut);
        maddr_non_byte++;
    }else{
      k--;
      curr_address_ut--;
    }
  }

  flt_cntr_loop: for (int i = 0; i < support_dim; i++)
  {
#pragma HLS PIPELINE
    faulty_addresses_cntr_local += faulty_addresses_cntr_support_array[i];
    faulty_addresses_cntr_support_array[i]=0;
  }
  
  first_fault_found=false;
*faulty_addresses_cntr=faulty_addresses_cntr_local;

}


/*****************************************************************************
 * @brief Read a single word of data and output on a stream 
 * and count the cc needed just 4 transfer  
 * NOT USED 
 * @deprecated
 *
 * @param[out]  sOutCmd the perf counter cmd stream
 * @param[out]  sOutreadData the read data stream
 * @param[in]   lcl_mem the virtual memory mapped pointer
 * @param[in]   max_addr_ut the maximum address to test
 * @param[in]   burst_size the run-time variable burst size
 *
 * @return Nothing.
 *****************************************************************************/
template <typename Tcntr, const unsigned int max_iterations=4000000,
const unsigned int buff_dim=64*2>
void pRDMainMemoryRead2StreamData(
  hls::stream<Tcntr>& sOutCmd,
  hls::stream<local_mem_word_t>& sOutreadData,
  membus_t * lcl_mem,
  local_mem_addr_t max_addr_ut,
    unsigned int burst_size)
{
#pragma HLS INLINE off

    local_mem_addr_t curr_address_ut;
    static local_mem_addr_t curr_reading_addr;
    static local_mem_word_t tmp_out[buff_dim];
#pragma HLS array_partition variable=tmp_out cyclic factor=2 dim=1

  sOutCmd.write(0);
  int i, reading_i;
  read_data_from_main_mem:
  for (i = 0, curr_address_ut = 0, curr_reading_addr=0, reading_i=0; curr_address_ut < max_addr_ut; )
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    if (!sOutreadData.full())
    {

      memcpy(tmp_out+reading_i, lcl_mem+curr_address_ut, sizeof(local_mem_word_t));
      if(reading_i > curr_reading_addr+1){
        sOutreadData.write(tmp_out[curr_reading_addr]);
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
        std::cout << " writing a memory word " << curr_reading_addr << " I have to reach " << i << std::endl;
      #endif
      #endif
        curr_reading_addr++;

      }
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
      std::cout << "read the " << reading_i << " memory word, outputreading at  " << curr_reading_addr << " i at " << i << std::endl;
      #endif
      #endif

      if(reading_i==buff_dim-1){
        reading_i=0;
      }else{
        reading_i++;
      }
      curr_address_ut++;
      i++;
    }
  }

  sOutCmd.write(0);
  i--;
  reading_i=i%buff_dim;
  sent_out_remaining_buff: 
  for (int j = 0; j < buff_dim; j++)
  {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = buff_dim
#pragma HLS PIPELINE II=1
    if (!sOutreadData.full())
    {
      if(j==curr_reading_addr && i >= curr_reading_addr){
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
        std::cout << " writing a memory word " << curr_reading_addr << " I have to reach " << i << std::endl;
      #endif
      #endif
        sOutreadData.write(tmp_out[curr_reading_addr]);
        curr_reading_addr++;
        tmp_out[j]=0;
      }else{
        tmp_out[j]=0;
      }
    }else{
      j--;
    }
  }
  curr_reading_addr=0;

}

/*****************************************************************************
 * @brief Read a variable burst_size amount of data and output on a stream 
 * and count the cc needed just 4 transfer  
 * THIS FUNCTION SUFFER FIFO OVERFLOW 4 transfer different from power of 2 numbers
 * NOT USED
 * @deprecated
 * @param[out]  sOutCmd the perf counter cmd stream
 * @param[out]  sOutreadData the read data stream
 * @param[in]   lcl_mem the virtual memory mapped pointer
 * @param[in]   max_addr_ut the maximum address to test
 * @param[in]   burst_size the run-time variable burst size
 *
 * @return Nothing.
 *****************************************************************************/
template <typename Tcntr, const unsigned int max_iterations=4000000,
const unsigned int buff_dim=64*2>
void pRDRead2StreamDataVariableBurst(
  hls::stream<Tcntr>& sOutCmd,
  hls::stream<local_mem_word_t>& sOutreadData,
  membus_t * lcl_mem,
  local_mem_addr_t max_addr_ut,
  unsigned int burst_size)
{
#pragma HLS INLINE off

    local_mem_addr_t curr_address_ut;
    static local_mem_word_t tmp_out[buff_dim];
#pragma HLS array_partition variable=tmp_out cyclic factor=2 dim=1
////TODO: Check if with 64 is better
    static local_mem_addr_t curr_reading_addr;
    static unsigned int end_distance=0;
    static int ptrs_distance=0;
    static int ptrs_distance_opposite=0;
    static bool transfer_less_than_burst = false;
    static bool activated_cntr = false;
    static bool can_read_data = false;

  read_data_from_main_mem:
  int reading_mm_i = 0;//how much filled the buff
  int consumed_fifo_i = 0;//how much already outputed
  int missing_words=0;
  unsigned int total_consumed_words=0;
  unsigned int total_readfrom_mm_words=0;
  bool fifo_is_not_full=false;
  for (curr_address_ut = 0, curr_reading_addr=0; curr_address_ut < max_addr_ut; )
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    if (!sOutreadData.full())
    {

      end_distance = max_addr_ut-curr_reading_addr;
      ptrs_distance = total_readfrom_mm_words - total_consumed_words;
      ptrs_distance_opposite = total_consumed_words - total_readfrom_mm_words;
      transfer_less_than_burst = burst_size>end_distance;
      fifo_is_not_full = ptrs_distance <= burst_size;
          //i have data to crunch, and eithre the fifo can read a burst or consumed enough to tx less than a burst)
      can_read_data=(end_distance > 0) && (fifo_is_not_full  || (ptrs_distance<=end_distance && transfer_less_than_burst));
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << "Max " << max_addr_ut << " reading at " << curr_reading_addr << " reading fifo at " << reading_mm_i <<  " consumed at " << consumed_fifo_i << std::endl;
          std::cout << "End dst " << end_distance << " ptrs dst " << ptrs_distance << " is last? " << transfer_less_than_burst << std::endl;
          std::cout << "curr_address_ut " << curr_address_ut << std::endl;  
      #endif
      #endif
      //if more than a burst size to available or the last iteration
      if(can_read_data){
        if (!transfer_less_than_burst)
        {
          //read a burst
          if(!activated_cntr){
            sOutCmd.write(0);
            activated_cntr = true;
          }else{
            sOutCmd.write(1);
          }
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << "BURST reading " << burst_size << " words from " << curr_reading_addr << " address, to  " << reading_mm_i << std::endl;
      #endif
      #endif
          memcpy(tmp_out+reading_mm_i, lcl_mem+curr_reading_addr, sizeof(local_mem_word_t)*burst_size);
          sOutCmd.write(1);
          std::cout << "before " << curr_reading_addr;
          curr_reading_addr+=burst_size;
          std::cout << " afterwards " << curr_reading_addr << std::endl;
          total_readfrom_mm_words+=burst_size;
          reading_mm_i=(reading_mm_i+burst_size)%buff_dim;
        }else{
          //read the missing words
          missing_words= end_distance%burst_size;
          std::cout << "before of before 1" << curr_reading_addr << std::endl;
          if(!activated_cntr){
            sOutCmd.write(0);
            activated_cntr = true;
          }else{
            sOutCmd.write(1);
          }
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << "LAST reading " << missing_words << " words from " << curr_reading_addr << " address, to  " << reading_mm_i << std::endl;
      #endif
      #endif
          total_readfrom_mm_words+=missing_words;
          std::cout << "before of before 3" << curr_reading_addr << std::endl;
          memcpy(tmp_out+reading_mm_i, lcl_mem+curr_reading_addr, sizeof(local_mem_word_t)*(end_distance%burst_size));
          sOutCmd.write(1);

          std::cout << "before " << curr_reading_addr;
          curr_reading_addr+=missing_words;
          std::cout << " afterwards " << curr_reading_addr << std::endl;
          reading_mm_i=(reading_mm_i+missing_words)%buff_dim;
        }
      }

      if(ptrs_distance > 0 || can_read_data){
        sOutreadData.write(tmp_out[consumed_fifo_i]);
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
        std::cout << " consumin a read memory word " << consumed_fifo_i << " I have to reach " << reading_mm_i << std::endl;
        std::cout << " The readmemoryword is  " << tmp_out[consumed_fifo_i]  << std::endl;
      #endif
     #endif

        if(consumed_fifo_i==buff_dim-1){
          consumed_fifo_i=0;
        }else{
          consumed_fifo_i++;
        }
        total_consumed_words++;
      }
      //// TODO: add this logic to end before the loop
      //// Example: with burst 64 the loop need less CC to consume everything
      //if() still data to read and to consume
      curr_address_ut++;
      //else{
      //  curr_address_ut=max_addr_ut;
      //}
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
        std::cout <<std::endl;
      #endif
     #endif
    }
  }
  sOutCmd.write(0);
  //reset
  end_distance=0;
  transfer_less_than_burst = false;
  activated_cntr=false;
  can_read_data=false;
  ptrs_distance=0;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////End of Read/ Functions//////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#endif //_ROLE_MEMTEST_PATTERN_LIBRARY_HPP_