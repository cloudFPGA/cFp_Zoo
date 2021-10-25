/*****************************************************************************
 * @file       memtest.cpp
 * @brief      The Role for a Memtest Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID, DCO
 * @date       September 2021
 *----------------------------------------------------------------------------
 *
 * @details      This application implements a UDP/TCP-oriented Memory test function.
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
#include "../include/memtest_pattern.hpp"
#include "../../../../../HOST/custom/memtest/languages/cplusplus/include/config.h" //debug level define

#ifdef USE_HLSLIB_DATAFLOW
#include "../../../../../hlslib/include/hlslib/xilinx/Stream.h"
#include "../../../../../hlslib/include/hlslib/xilinx/Simulation.h"
#endif

#ifdef USE_HLSLIB_STREAM
using hlslib::Stream;
#endif
using hls::stream;

#define Data_t_in  ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0>
#define Data_t_out ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0>

PacketFsmType enqueueFSM = WAIT_FOR_META;
PacketFsmType dequeueFSM = WAIT_FOR_META;

typedef char word_t[8];


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
    //#pragma HLS DATAFLOW interval=1
     #pragma  HLS INLINE 
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord    netWord;
    ap_uint<16> max_iterations;
    static bool start_stop_local = false;
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
      #if DEBUG_LEVEL == TRACE_ALL
	    printf("Hallo, I received a stop command D:\n");
      #endif
            break;
          default:
            if (start_stop_local)
            {
              //some data manipulation here
              // everything is running and should no sending anything back
            } else {
              netWord.tdata=TEST_INVLD_CMD;
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
    #pragma  HLS INLINE
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord      netWordTx;
    NetworkMeta  meta_in = NetworkMeta();
    static NodeId dst_rank;
  
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
    
const unsigned int ddr_mem_depth = TOTMEMDW_512*2;
const unsigned int ddr_latency = DDR_LATENCY;
const unsigned int MAX_BURST_LENGTH_512=64;

// Mapping LCL_MEM0 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem0 bundle=moMEM_Mp1\
  max_read_burst_length=MAX_BURST_LENGTH_512  max_write_burst_length=MAX_BURST_LENGTH_512 offset=direct \
  num_read_outstanding=16 num_write_outstanding=16 latency=ddr_latency

// Mapping LCL_MEM1 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem1 bundle=moMEM_Mp1 \
  max_read_burst_length=MAX_BURST_LENGTH_512  max_write_burst_length=MAX_BURST_LENGTH_512 offset=direct \
  num_read_outstanding=16 num_write_outstanding=16 latency=ddr_latency

#endif

  //-- LOCAL VARIABLES ------------------------------------------------------
  NetworkMetaStream  meta_tmp = NetworkMetaStream();
  static stream<NetworkMetaStream> sRxtoProc_Meta("sRxtoProc_Meta");
  static stream<NetworkMetaStream> sProctoTx_Meta("sProctoTx_Meta");
  static stream<NetworkWord>       sProcpToTxp_Data("sProcpToTxp_Data"); 
 #pragma HLS STREAM variable=sProcpToTxp_Data depth=max_proc_fifo_depth dim=1
  static hls::stream<bool> sOfEnableCCIncrement("sOfEnableCCIncrement");
 #pragma HLS STREAM variable=sOfEnableCCIncrement depth=20 dim=1
  static hls::stream<bool> sOfResetCounter("sOfResetCounter");
 #pragma HLS STREAM variable=sOfResetCounter depth=20 dim=1
  static hls::stream<bool> sOfGetTheCounter("sOfGetTheCounter");
 #pragma HLS STREAM variable=sOfGetTheCounter depth=20 dim=1
  static hls::stream<ap_uint<64>> sClockCounter("sClockCounter");
 #pragma HLS STREAM variable=sClockCounter depth=20 dim=1

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
#pragma HLS reset variable=enqueueFSM
#pragma HLS reset variable=dequeueFSM
#pragma HLS reset variable=processed_word_rx
#pragma HLS reset variable=processed_word_tx

  

#ifdef USE_HLSLIB_DATAFLOW //TODO: this is not used currently and not updated, consider to cut out
  /*! @copybrief uppercase()
   *  Uppercase is eanbled with hlslib support
   */
  /*! @copydoc uppercase()
   * Use this snippet to early check for C++ errors related to dataflow and bounded streams (empty 
   * and full) during simulation. It can also be both synthesized and used in co-simulation.
   * Practically we use hlslib when we want to run simulation as close as possible to the HW, by 
   * executing all functions of dataflow in thread-safe parallel executions, i.e the function 
   * HLSLIB_DATAFLOW_FINALIZE() acts as a barrier for the threads spawned to serve every function 
   * called in HLSLIB_DATAFLOW_FUNCTION(func, args...).
   */
   /*! @copydetails uppercase()
   * hlslib is a collection of C++ headers, CMake files, and examples, aimed at improving the 
   * quality of life of HLS developers. More info at: https://github.com/definelicht/hlslib
   */
  // Dataflow functions running in parallel
  HLSLIB_DATAFLOW_INIT();
  
  HLSLIB_DATAFLOW_FUNCTION(pRXPath, 
         siSHL_This_Data,
         siNrc_meta,
         sRxtoTx_Meta,
         meta_tmp,
         sRxpToProcp_Data,
         &processed_word_rx,
         &processed_bytes_rx);

  HLSLIB_DATAFLOW_FUNCTION(pTXPath,
         soTHIS_Shl_Data,
         soNrc_meta,
         sRxpToProcp_Data,
         sRxtoTx_Meta,
         &processed_word_tx,
         pi_rank,
         pi_size);

  HLSLIB_DATAFLOW_FINALIZE();
  
#else // !USE_HLSLIB_DATAFLOW
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
//STEP 2.a: processing the data. 
// INSERT THE CUSTOM PROCESSING LOGIC HERE
//////////////////////////////////////////////////
 pTHISProcessingData<bool,64>(
  sRxpToProcp_Data,
  sProcpToTxp_Data,
  sRxtoProc_Meta,
  sProctoTx_Meta,
  &start_stop//,
  //sOfEnableCCIncrement,
  //sOfResetCounter,
  //sOfGetTheCounter,
  //sClockCounter,
  //sPerfCounter_cmd,
  //sPerfCounter_results
  #ifdef ENABLE_DDR
              ,
    lcl_mem0,
    lcl_mem1
  #endif
  );

//////////////////////////////////////////////////
// STEP 2.b: Hardware Performance Counter
// it runs in parallel/coordinated with STEP 2.a
//////////////////////////////////////////////////
  // pCountClockCycles<bool,64,4000000>(
  //   sOfEnableCCIncrement,
  //   sOfResetCounter,
  //   sOfGetTheCounter,
  //   sClockCounter);

// perfCounterProc<ap_uint<64>,ap_uint<64>,64 >(
//   sPerfCounter_cmd,
//   sPerfCounter_results, 
//   0,
//   256,
//   16);
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
#endif // USE_HLSLIB_DATAFLOW
}


/*! \} */
