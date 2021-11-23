/*****************************************************************************
 * @file       sobel_hw_common.hpp
 * @brief      A library for some common functionalities:
 *              Memory interaction
 *              Performance Counters
 * @author     FAB, WEI, NGL, DID, DCO
 * @date       September 2021
 *----------------------------------------------------------------------------
 *
 * @details  Implementations of library example functionalities:
 * 
 * Memory interaction 
 *    pMyMemtestMemCpy --> memcpy reimplementation
 *    pMemCpyCircularBuff --> memcpy using a circular buffer (not optimized)
 *    pReadAxiMemMapped2HlsStream --> read data and write on stream
 *    pReadAxiMemMapped2HlsStreamCountFirst --> as before but with the activation of perf counter
 *    pReadAxiMemMapped2HlsStreamCountActivated --> as before but with perf counter activated
 *    pReadAxiMemMapped2HlsStreamCountExtern --> summary of two function before but with activation as input param
 * 
 * Performance Counters
 *    perfCounterProc --> original function from Xilinx for perf counters that takes INIT and STOP (any)
 *    perfCounterProc2Mem --> first iteration on the original function
 *    perfCounterProc2MemCountOnly --> trimming unsued stuffs
 *    perfCounterProc2MemCountIncremental --> as before but incrementing the output register not overwriting
 *    perfCounterMultipleCounts --> as before but with multiple activations possible: INIT;ANY(repeat);STOP(0)
 *    pCountClockCycles --> alternative implementations that does not behave correctly in hls
 * 
 * @deprecated pMemCpyCircularBuff; pCountClockCycles
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup SobelHLS
 * @addtogroup SobelHLS
 * \{
 *****************************************************************************/

#ifndef _ROLE_SOBEL_HW_COMMON_HPP_
#define _ROLE_SOBEL_HW_COMMON_HPP_

#include <stdio.h>
#include <iostream>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>
#include "../../../../../HOST/vision/sobel/languages/cplusplus/include/config.h"//debug level define 
#include "memory_utils.hpp" //for stream based communication with ddr
#include "network.hpp"

using namespace hls;

//////////////////////////////////////////////////////////////////////////////
//////////////////Begin of Mem. Interaction Functions/////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * @brief Copy a fixed compile time amount of data to another array
 *
 * @param[out] out the dst ptr
 * @param[in]  in the src ptr
 * @param[in]  Tin the input datatype
 * @param[in]  Tout the output datatype
 * @param[in]  arraysize the fixed amount of data
 *
 * @return Nothing.
 *****************************************************************************/
template<typename Tin, typename Tout, unsigned int arraysize>
void pMyMemtestMemCpy(Tin* in, Tout* out){
#pragma HLS INLINE
  for (unsigned int i = 0; i < arraysize; i++)
  {
#pragma HLS PIPELINE II=1
    *out = *in;
  }
  
}

/*****************************************************************************
 * @brief Copy a run-time variable amount of data to another array employing
 *  the src as circular buffer i.e.,  handling overflow
 *
 * @param[out] out_mem the dst ptr
 * @param[in]  buff the src ptr, or the circular buffer
 * @param[in]  elems the current amount of data to tx
 * @param[in]  offset_buff the initial offest in the circular buffer
 * @param[in]  Tin the input datatype
 * @param[in]  Tout the output datatype
 * @param[in]  arraysize the maxmimum amount of data
 *
 * @return Nothing.
 *****************************************************************************/
template<typename Tin, typename Tout, const unsigned int arraysize>
void pMemCpyCircularBuff(Tin* buff, Tout* out_mem, unsigned int elems,unsigned int offset_buff){
#pragma HLS INLINE
  unsigned int j = 0;
  circ_buff_loop: for (unsigned int i = 0; i < elems; i++)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = arraysize
    if(offset_buff+j==arraysize)//
    { 
      offset_buff=0;
      j=1;
      out_mem[i] = buff[0];
    }else{
      out_mem[i] = buff[offset_buff+j];
      j++;
    }
  }
  
}


/*****************************************************************************
 * @brief Copy a run-time variable amount of data to an hls stream with a given max
 *
 * @param[out] main_mem the src ptr to read
 * @param[in]  sOut the dst hls stream
 * @param[in]  elems the current amount of data to tx
 * @param[in]  Tin the input datatype
 * @param[in]  Tout the output datatype
 * @param[in]  burstsize the maxmimum amount of data
 *
 * @return Nothing.
 *****************************************************************************/
template<typename Tin, typename Tout, const unsigned int burstsize>
void pReadAxiMemMapped2HlsStream(Tin* main_mem, hls::stream<Tout> &sOut, unsigned int elems){
#pragma HLS INLINE
  mmloop: for (unsigned int i = 0; i < elems; i++)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = burstsize
    Tout tmp  = main_mem[i];
    sOut.write(tmp);
  }
  
}

/*****************************************************************************
 * @brief Copy a run-time variable amount of data to an hls stream with a given max
 *  it assumes also the initialization of a perf counter of "perfCounterMultipleCounts" 
 *  function
 *
 * @param[out] main_mem the src ptr to read
 * @param[in]  sOut the dst hls stream
 * @param[in]  elems the current amount of data to tx
 * @param[in]  cmd the performance counter cmd stream
 * @param[in]  Tin the input datatype
 * @param[in]  Tout the output datatype
 * @param[in]  burstsize the maxmimum amount of data
 * @param[in]  Tcntr the cmd perf counter datatype
 *
 * @return Nothing.
 *****************************************************************************/
template<typename Tin, typename Tout, const unsigned int burstsize, typename Tcntr>
void pReadAxiMemMapped2HlsStreamCountFirst(Tin* main_mem, hls::stream<Tout> &sOut, unsigned int elems, hls::stream<Tcntr>& cmd){
#pragma HLS INLINE
cmd.write(0);
  mmloop: for (unsigned int i = 0; i < elems; i++)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = burstsize
    Tout tmp  = main_mem[i];
    sOut.write(tmp);
  }
  cmd.write(1);
  
}

/*****************************************************************************
 * @brief Copy a run-time variable amount of data to an hls stream with a given max
 *  it assumes  "perfCounterMultipleCounts" function already initialized so it just incr
 *
 * @param[out] main_mem the src ptr to read
 * @param[in]  sOut the dst hls stream
 * @param[in]  elems the current amount of data to tx
 * @param[in]  cmd the performance counter cmd stream
 * @param[in]  Tin the input datatype
 * @param[in]  Tout the output datatype
 * @param[in]  burstsize the maxmimum amount of data
 * @param[in]  Tcntr the cmd perf counter datatype
 *
 * @return Nothing.
 *****************************************************************************/
template<typename Tin, typename Tout, const unsigned int burstsize, typename Tcntr>
void pReadAxiMemMapped2HlsStreamCountActivated(Tin* main_mem, hls::stream<Tout> &sOut, unsigned int elems, hls::stream<Tcntr>& cmd){
#pragma HLS INLINE
  cmd.write(1);
  mmloop: for (unsigned int i = 0; i < elems; i++)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = burstsize
    Tout tmp  = main_mem[i];
    sOut.write(tmp);
  }
  cmd.write(1);
}



/*****************************************************************************
 * @brief   Store a net word to a local AXI stream
 * @return Nothing.
 *****************************************************************************/
template<typename Tin,const unsigned int loop_cnt,
const unsigned int bytes_per_loop, const unsigned int max_data_transfer>
void storeWordToAxiStream(
  NetworkWord word,
  // stream<ap_uint<INPUT_PTR_WIDTH>>    &img_in_axi_stream,
  Tin    &img_in_axi_stream,
  unsigned int                      *processed_word_rx,
  unsigned int                      *processed_bytes_rx,
  stream<bool>                      &sImageLoaded    
)
{   
  #pragma HLS INLINE
  Data_t_in v;
  // const unsigned int loop_cnt = (BITS_PER_10GBITETHRNET_AXI_PACKET/INPUT_PTR_WIDTH);
  // const unsigned int bytes_per_loop = (BYTES_PER_10GBITETHRNET_AXI_PACKET/loop_cnt);
  unsigned int bytes_with_keep = 0;
  for (unsigned int i=0; i<loop_cnt; i++) {
    if ((word.tkeep >> i) == 0) {
      printf("WARNING: value with tkeep=0 at i=%u\n", i);
      continue; 
    }
    v.data = (ap_uint<INPUT_PTR_WIDTH>)(word.tdata >> i*8);
    v.keep = word.tkeep;
    v.last = word.tlast;
    img_in_axi_stream.write(v.data);
    bytes_with_keep += bytes_per_loop;
  }
  if (*processed_bytes_rx < max_data_transfer){
  //  IMGSIZE-BYTES_PER_10GBITETHRNET_AXI_PACKET) {
    (*processed_bytes_rx) += bytes_with_keep;
    if (!sImageLoaded.full()) {
        sImageLoaded.write(false);
    }
  }
  else {
    printf("DEBUG in storeWordToAxiStream: WARNING - you've reached the max depth of img. Will put *processed_bytes_rx = 0.\n");
    *processed_bytes_rx = 0;
    if (!sImageLoaded.full()) {
        sImageLoaded.write(true);
    }
  }
}

/*****************************************************************************
 * @brief   Store a net word to local memory
 * @return Nothing.
 *****************************************************************************/
template<typename TInImg, const unsigned int img_pckts>
// void storeWordToArray(uint64_t input, ap_uint<INPUT_PTR_WIDTH> img[IMG_PACKETS], 
void storeWordToArray(uint64_t input, TInImg img[img_pckts], 
		      unsigned int *processed_word, unsigned int *image_loaded)
{
  #pragma HLS INLINE
  
  img[*processed_word] = (TInImg) input;
  printf("DEBUG in storeWordToArray: input = %u = 0x%16.16llX \n", input, input);
  printf("DEBUG in storeWordToArray: img[%u]= %u = 0x%16.16llX \n", *processed_word, 
  (uint64_t)img[*processed_word], (uint64_t)img[*processed_word]);
  if (*processed_word < img_pckts-1) {
    *processed_word++;
  }
  else {
    printf("DEBUG in storeWordToArray: WARNING - you've reached the max depth of img[%u]. Will put *processed_word = 0.\n", *processed_word);
    *processed_word = 0;
    *image_loaded = 1;
  }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////End of Mem. Interaction Functions///////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//////////////////Begin of Perf. Counter Functions////////////////////////////
//////////////////////////////////////////////////////////////////////////////

const unsigned long int  max_counter_cc = 4000000;

//Original function from Xilinx Vitis Accel examples, template from DCO
// @DEPRECATED
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

//Original function from Xilinx Vitis Accel examples, template from DCO
// @DEPRECATED
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


/*****************************************************************************
 * @brief Count Clock Cycles between two events, the first event init the 
 * counter the second stop the count
 *
 * @param[in]  cmd the performance counter cmd stream, first is init second stop
 * @param[out] out the output register of where store the counter value
 * @param[in]  Tin the input datatype
 * @param[in]  Tout the output datatype
 * @param[in]  counter_precision the maxmimum amount of data
 *
 * @return Nothing.
 *****************************************************************************/
template<typename Tin, typename Tout, unsigned int counter_precision=64>
void perfCounterProc2MemCountOnly(hls::stream<Tin>& cmd, Tout * out) {
  
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


/*****************************************************************************
 * @brief Count Clock Cycles between two events, the first event init the 
 * counter the second stop the count and increment the out register
 * TODO: seems not working at the csim lvl (never tested below) when executing single
 * DUT step, hanging stream values
 *
 * @param[in]  cmd the performance counter cmd stream, first is init second stop
 * @param[out] out the output register of where increment the counter value
 * @param[in]  Tin the input datatype
 * @param[in]  Tout the output datatype
 * @param[in]  counter_precision the maxmimum amount of data
 *
 * @return Nothing.
 *****************************************************************************/
template<typename Tin, typename Tout, unsigned int counter_precision=64>
void perfCounterProc2MemCountIncremental(hls::stream<Tin>& cmd, Tout * out) {
  
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
    *out +=cnt;
}

/*****************************************************************************
 * @brief Count Clock Cycles between two events, the first event init the 
 * counter the second stop the count, a 0 after the init stop definitevely the counter
 *
 * @param[in]  cmd the performance counter cmd stream, first is init second stop(0)/continue(everything else)
 * @param[out] out the output register of where store the incremental counter value
 * @param[in]  Tin the input datatype
 * @param[in]  Tout the output datatype
 * @param[in]  counter_precision the maxmimum amount of data
 *
 * @return Nothing.
 *****************************************************************************/
template<typename Tin, typename Tout, unsigned int counter_precision=64>
void perfCounterMultipleCounts(hls::stream<Tin>& cmd, Tout * out) {
  #pragma HLS interface ap_ctrl_none port=return
    Tin input_cmd=1;

    // wait to receive a value to start counting
    ap_uint<counter_precision> cnt = cmd.read();
    reset:
    while (input_cmd != 0)//a zero will stop the counter
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_counter_cc
#if DEBUG_LEVEL == TRACE_ALL
 #ifndef __SYNTHESIS__
  //printf("DEBUG begin to count :D input_cmd value = %s\n", input_cmd.to_string().c_str());
#endif //__SYNTHESIS__
#endif     
// keep counting until a value is available
count:
    while (cmd.read_nb(input_cmd) == false) {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_counter_cc
#pragma HLS PIPELINE II=1
        cnt++;       
#if DEBUG_LEVEL == TRACE_ALL
 #ifndef __SYNTHESIS__
 // printf("DEBUG perfCounterProc counter value = %s\n", cnt.to_string().c_str());
#endif //__SYNTHESIS__
#endif     
    }
    input_cmd=cmd.read();
  }
  *out +=cnt;
}

/*****************************************************************************
 * @brief Count Clock Cycles between two events first sketch
 * TODO: make it working without counting with the stream or reshaping as FSM
 *
 * @param[in]  sOfEnableCCIncrement 
 * @param[in]  sOfResetCounter 
 * @param[in]  sOfGetTheCounter 
 * @param[in]  oSClockCounter 
 * @param[in]  Tevent the event datatype
 * @param[in]  counter_width the counter precision
 * @param[in]  maximum_counter_value_before_reset the maxmimum amount of cc count before auto reset
 *
 * @return Nothing.
 *****************************************************************************/
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
//////////////////////////////////////////////////////////////////////////////
//////////////////End of Perf. Counter Functions//////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#endif //_ROLE_SOBEL_HW_COMMON_HPP_