/*****************************************************************************
 * @file       memtest.cpp
 * @brief      The Role for a Memtest Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID, DCO
 * @date       September 2021
 *----------------------------------------------------------------------------
 *
 * @details      This application implements a UDP/TCP-oriented Memory test function.
 * Mainly structural component (like a hw top)
 *
 * @deprecated   For the time being, we continue designing with the DEPRECATED
 *               directives because the new PRAGMAs do not work for us.
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup MemtestHLS
 * @addtogroup MemtestHLS
 * \{
 *****************************************************************************/

#include "../include/memtest.hpp"
#include "../include/memtest_processing.hpp"
#include "../include/memtest_pattern_library.hpp"
#include "../include/memtest_library.hpp"
#include "../../../../../HOST/custom/memtest/languages/cplusplus/include/config.h" //debug level define

#ifdef USE_HLSLIB_STREAM
using hlslib::Stream;
#endif
using hls::stream;

#define Data_t_in  ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0>
#define Data_t_out ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0>


/*****************************************************************************
 * @brief   Main process of the Memtest Application 
 * directives.
 * @deprecated  This functions is using deprecated AXI stream interface 
 * @return Nothing.
 *****************************************************************************/
void memtest(

    ap_uint<32>             *pi_rank,
    ap_uint<32>             *pi_size,
    //------------------------------------------------------
    //-- SHELL / This / Udp/TCP Interfaces
    //------------------------------------------------------
    stream<NetworkWord>         &siSHL_This_Data,
    stream<NetworkWord>         &soTHIS_Shl_Data,
    stream<NetworkMetaStream>   &siNrc_meta,
    stream<NetworkMetaStream>   &soNrc_meta,
    ap_uint<32>                 *po_rx_ports
    #ifdef ENABLE_DDR
                                            ,
    //------------------------------------------------------
    //-- SHELL / Role / Mem / Mp1 Interface
    //------------------------------------------------------    
    membus_t                    *lcl_mem0,
    membus_t                    *lcl_mem1
    #endif
    
    )
{
  //-- DIRECTIVES FOR THE BLOCK ---------------------------------------------
 //#pragma HLS INTERFACE ap_ctrl_none port=return

  //#pragma HLS INTERFACE ap_stable     port=piSHL_This_MmioEchoCtrl

#pragma HLS INTERFACE axis register both port=siSHL_This_Data
#pragma HLS INTERFACE axis register both port=soTHIS_Shl_Data

#pragma HLS INTERFACE axis register both port=siNrc_meta
#pragma HLS INTERFACE axis register both port=soNrc_meta

#pragma HLS INTERFACE ap_ovld register port=po_rx_ports name=poROL_NRC_Rx_ports
#pragma HLS INTERFACE ap_stable register port=pi_rank name=piFMC_ROL_rank
#pragma HLS INTERFACE ap_stable register port=pi_size name=piFMC_ROL_size


#ifdef ENABLE_DDR
    
const unsigned int ddr_mem_depth = TOTMEMDW_512;//*2;
const unsigned int ddr_latency = DDR_LATENCY;
const unsigned int num_outstanding_transactions = 256;//16;
const unsigned int MAX_BURST_LENGTH_512=64;//Theoretically is  64, 64*512bit = 4096KBytes;

// Mapping LCL_MEM0 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem0 bundle=moMEM_Mp1\
  max_read_burst_length=MAX_BURST_LENGTH_512  max_write_burst_length=MAX_BURST_LENGTH_512 offset=direct \
  num_read_outstanding=num_outstanding_transactions num_write_outstanding=num_outstanding_transactions latency=ddr_latency

// Mapping LCL_MEM1 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem1 bundle=moMEM_Mp1 \
  max_read_burst_length=MAX_BURST_LENGTH_512  max_write_burst_length=MAX_BURST_LENGTH_512 offset=direct \
  num_read_outstanding=num_outstanding_transactions num_write_outstanding=num_outstanding_transactions latency=ddr_latency

#endif

  //-- LOCAL VARIABLES ------------------------------------------------------
  NetworkMetaStream  meta_tmp = NetworkMetaStream();
  static stream<NetworkMetaStream> sRxtoProc_Meta("sRxtoProc_Meta");
  static stream<NetworkMetaStream> sProctoTx_Meta("sProctoTx_Meta");
  static stream<NetworkWord>       sProcpToTxp_Data("sProcpToTxp_Data"); 
 #pragma HLS STREAM variable=sProcpToTxp_Data depth=20 dim=1
  static stream<NetworkWord>       sRxpToProcp_Data("sRxpToProcp_Data");

 static hls::stream<ap_uint<64>> sPerfCounter_cmd("sPerfCounter_cmd"); 
 #pragma HLS STREAM variable=sPerfCounter_cmd depth=1 dim=1
 static hls::stream<ap_uint<64>> sPerfCounter_results("sPerfCounter_results"); 
 #pragma HLS STREAM variable=sPerfCounter_results depth=2 dim=1 //contain  ALL the output of this process

  static unsigned int processed_word_rx;
  static unsigned int processed_bytes_rx;
  static unsigned int processed_word_tx;
  //*po_rx_ports = 0x1; //currently work only with default ports...
  static stream<NodeId>            sDstNode_sig   ("sDstNode_sig");
  bool                              start_stop;
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS DATAFLOW
#pragma HLS reset variable=processed_word_rx
#pragma HLS reset variable=processed_word_tx

  
//////////////////////////////////////////////////
//STEP 0: setup the port and the dst of the cluster
// CHANGE THE CLUSTER CONNECTIONS HERE
//////////////////////////////////////////////////
 pPortAndDestionation(
  pi_rank,
  pi_size, 
  sDstNode_sig,
  po_rx_ports);

//////////////////////////////////////////////////
//STEP 1: received the input data, small parse on 
// the command and fwd to the following step
// CHANGE THE COMMAND PARSING HERE
//////////////////////////////////////////////////
 pRXPath(
  siSHL_This_Data,
  siNrc_meta,
  sRxtoProc_Meta,
  sRxpToProcp_Data,
  meta_tmp,
  &start_stop,
  &processed_word_rx,
  &processed_bytes_rx);

//////////////////////////////////////////////////
//STEP 2: processing the data. 
// INSERT THE CUSTOM PROCESSING LOGIC HERE
//////////////////////////////////////////////////
 pTHISProcessingData<64>(
  sRxpToProcp_Data,
  sProcpToTxp_Data,
  sRxtoProc_Meta,
  sProctoTx_Meta,
  &start_stop
  #ifdef ENABLE_DDR
              ,
    lcl_mem0,
    lcl_mem1
  #endif
  );

//////////////////////////////////////////////////
// STEP 3: transmit back the data
// currently steup the tlast once reached max size
// WARNING: it needs a new meta if filled up the MTU
//////////////////////////////////////////////////
  pTXPath(
  soTHIS_Shl_Data,
  soNrc_meta,
  sProcpToTxp_Data,
  sProctoTx_Meta,
  sDstNode_sig,
  &processed_word_tx,
  pi_rank);


}
/*! \} */
