/*****************************************************************************
 * @file       memtest_library.hpp
 * @brief      A library for some common functionalities
 * @author     FAB, WEI, NGL, DID, DCO
 * @date       September 2021
 *----------------------------------------------------------------------------
 *
 * @details      This application implements a UDP/TCP-oriented Memory test function.
 *
 * @deprecated   
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup MemtestHLS
 * @addtogroup MemtestHLS
 * \{
 *****************************************************************************/

#ifndef _ROLE_MEMTEST_LIBRARY_HPP_
#define _ROLE_MEMTEST_LIBRARY_HPP_

#include <stdio.h>
#include <iostream>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>
#include "../../../../../HOST/custom/memtest/languages/cplusplus/include/config.h"//debug level define 

#include "network.hpp"

using namespace hls;

//////////////////////////////////////////////////////////////////////////////
//////////////////Begin of Network-Related Functions//////////////////////////
//////////////////////////////////////////////////////////////////////////////

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


/*****************************************************************************
 * @brief Receive Path - From SHELL to THIS.
 *
 * @param[in]  siSHL_This_Data
 * @param[in]  siNrc_meta
 * @param[out] sRxtoTx_Meta
 * @param[out] sRxpToProcp_Data
 * @param[out] start_stop
 * @param[out] meta_tmp
 * @param[out] processed_word
 *
 * @return Nothing.
 ******************************************************************************/
void pRXPath(
  stream<NetworkWord>                              &siSHL_This_Data,
  stream<NetworkMetaStream>                        &siNrc_meta,
  stream<NetworkMetaStream>                        &sRxtoProc_Meta,
  stream<NetworkWord>                              &sRxpToProcp_Data,
  NetworkMetaStream                                meta_tmp,
  bool  *                                           start_stop,
  unsigned int                                     *processed_word_rx,
  unsigned int                                     *processed_bytes_rx
      )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
     #pragma  HLS INLINE  off
    //-- LOCAL VARIABLES ------------------------------------------------------


static PacketFsmType enqueueFSM = WAIT_FOR_META;
#pragma HLS reset variable=enqueueFSM


    NetworkWord    netWord;
    ap_uint<16> max_iterations;
    static bool start_stop_local = false;
    static bool prev_was_start = false;
#pragma HLS reset variable=start_stop_local

    *start_stop = start_stop_local;
  switch(enqueueFSM)
  {
    case WAIT_FOR_META: 
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG in pRXPath: enqueueFSM - WAIT_FOR_META, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
       *processed_word_rx, *processed_bytes_rx);
    #endif
      if ( !siNrc_meta.empty() && !sRxtoProc_Meta.full() )
      {
        meta_tmp = siNrc_meta.read();//not sure if I have to continue to test or not, hence sending the meta or not is different
        meta_tmp.tlast = 1; //just to be sure...
        sRxtoProc_Meta.write(meta_tmp); //valid destination
        enqueueFSM = PROCESSING_PACKET;
      }
      break;

    case PROCESSING_PACKET:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG in pRXPath: enqueueFSM - PROCESSING_PACKET, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
       *processed_word_rx, *processed_bytes_rx);
    #endif
      
      if ( !siSHL_This_Data.empty() && !sRxpToProcp_Data.full() )
      {
        //-- Read incoming data chunk
        netWord = siSHL_This_Data.read();

        switch(netWord.tdata.range(MEMTEST_COMMANDS_HIGH_BIT,MEMTEST_COMMANDS_LOW_BIT))//the command is in the first two bits
        {
          case(TEST_START_CMD):
            start_stop_local=true;
            *start_stop=true;
            sRxpToProcp_Data.write(netWord);
            prev_was_start=true;
      #if DEBUG_LEVEL == TRACE_ALL
	    printf("Hallo, I received a start command :D\n");
      #endif
            break;
          case(TEST_STOP_CMD):
            start_stop_local=false;
            *start_stop=false;
            netWord.tdata=TEST_STOP_CMD;
            netWord.tlast = 1;
            sRxpToProcp_Data.write(netWord);
            prev_was_start=false;
      #if DEBUG_LEVEL == TRACE_ALL
	    printf("Hallo, I received a stop command D:\n");
      #endif
            break;

          case(TEST_BURSTSIZE_CMD):
      #if DEBUG_LEVEL == TRACE_ALL
      printf("Hallo, I received a burst size command :), and prev_was_start=%u\n",prev_was_start);
      #endif
            if (prev_was_start)
            {
              sRxpToProcp_Data.write(netWord);

            }else{
              netWord.tdata=TEST_INVLD_CMD;
              sRxpToProcp_Data.write(netWord);
            }
            prev_was_start=false;
            break;
          default:
            if (start_stop_local)
            {
              //some data manipulation here
              // everything is running and should no sending anything back
            } else {
              netWord.tdata=TEST_INVLD_CMD;
              prev_was_start=false;
              sRxpToProcp_Data.write(netWord);
            }
            break;

        } 
        //no need to forwarding every packet to the processing, hence commenting out
        //sRxpToProcp_Data.write(netWord);
        if(netWord.tlast == 1)
        {
          enqueueFSM = WAIT_FOR_META;
        }
      }
      break;
  }

 
}


/*****************************************************************************
 * @brief Transmit Path - From THIS to SHELL.
 *
 * @param[out] soTHIS_Shl_Data
 * @param[out] soNrc_meta
 * @param[in]  sProcpToTxp_Data
 * @param[in]  sRxtoTx_Meta
 * @param[in]  pi_rank
 * @param[in]  sDstNode_sig
 *
 * @return Nothing.
 *****************************************************************************/
void pTXPath(
  stream<NetworkWord>         &soTHIS_Shl_Data,
  stream<NetworkMetaStream>   &soNrc_meta,
  stream<NetworkWord>         &sProcpToTxp_Data,
  stream<NetworkMetaStream>   &sRxtoTx_Meta,
  stream<NodeId>              &sDstNode_sig,
  unsigned int                *processed_word_tx, 
  ap_uint<32>                 *pi_rank
)
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    //#pragma HLS DATAFLOW interval=1
    #pragma  HLS INLINE off
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord      netWordTx;
    NetworkMeta  meta_in = NetworkMeta();
    static NodeId dst_rank;
    static PacketFsmType dequeueFSM = WAIT_FOR_META;
    #pragma HLS reset variable=dequeueFSM
  
  switch(dequeueFSM)
  {
    case WAIT_FOR_META:
      if(!sDstNode_sig.empty())
      {
        dst_rank = sDstNode_sig.read();
        dequeueFSM = WAIT_FOR_STREAM_PAIR;
        //Triangle app needs to be reset to process new rank
      }
      break;
    case WAIT_FOR_STREAM_PAIR:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG in pTXPath: dequeueFSM=%d - WAIT_FOR_STREAM_PAIR, *processed_word_tx=%u\n", 
       dequeueFSM, *processed_word_tx);
    #endif
      //-- Forward incoming chunk to SHELL
      *processed_word_tx = 0;

      if (( !sProcpToTxp_Data.empty() && !sRxtoTx_Meta.empty() 
          && !soTHIS_Shl_Data.full() &&  !soNrc_meta.full() )) 
      {
        netWordTx = sProcpToTxp_Data.read();

  // in case MTU=8 ensure tlast is set in WAIT_FOR_STREAM_PAIR and don't visit PROCESSING_PACKET
  if (PACK_SIZE == 8) 
  {
      netWordTx.tlast = 1;
  }
        soTHIS_Shl_Data.write(netWordTx);

        meta_in = sRxtoTx_Meta.read().tdata;
        NetworkMetaStream meta_out_stream = NetworkMetaStream();
        meta_out_stream.tlast = 1;
        meta_out_stream.tkeep = 0xFF; //just to be sure

        meta_out_stream.tdata.dst_rank = dst_rank;
        meta_out_stream.tdata.src_rank = (NodeId) *pi_rank;
        meta_out_stream.tdata.dst_port = meta_in.src_port;
        meta_out_stream.tdata.src_port = meta_in.dst_port;

        soNrc_meta.write(meta_out_stream);

        (*processed_word_tx)++;
  
        if(netWordTx.tlast != 1)
        {
          dequeueFSM = PROCESSING_PACKET;
        }
      }
      break;

    case PROCESSING_PACKET: 
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG in pTXPath: dequeueFSM=%d - PROCESSING_PACKET, *processed_word_tx=%u\n", 
       dequeueFSM, *processed_word_tx);
    #endif
      if( !sProcpToTxp_Data.empty() && !soTHIS_Shl_Data.full())
      {
        netWordTx = sProcpToTxp_Data.read();

  // This is a normal termination of the axi stream from vitis functions
  if(netWordTx.tlast == 1)
  {
    dequeueFSM = WAIT_FOR_STREAM_PAIR;
  }
  
  // This is our own termination based on the custom MTU we have set in PACK_SIZE.
  // TODO: We can map PACK_SIZE to a dynamically assigned value either through MMIO or header
  //       in order to have a functional bitstream for any MTU size
  (*processed_word_tx)++;
  if (((*processed_word_tx)*8) % PACK_SIZE == 0) 
  {
      netWordTx.tlast = 1;
      dequeueFSM = WAIT_FOR_STREAM_PAIR;
  }
  
        soTHIS_Shl_Data.write(netWordTx);
      }
      break;
  }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////End of Network-Related Functions////////////////////////////
//////////////////////////////////////////////////////////////////////////////

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
void pReadAxiMemMapped2HlsStreamCountExtern(Tin* main_mem, hls::stream<Tout> &sOut, unsigned int elems, hls::stream<Tcntr>& cmd, bool activated){
#pragma HLS INLINE
  cmd.write(activated);
  mmloop: for (unsigned int i = 0; i < elems; i++)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = burstsize
    Tout tmp  = main_mem[i];
    sOut.write(tmp);
  }
  cmd.write(1);
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

#endif //_ROLE_MEMTEST_LIBRARY_HPP_