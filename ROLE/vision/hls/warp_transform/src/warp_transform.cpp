/*****************************************************************************
 * @file       warp_transform.cpp
 * @brief      The Role for a WarpTransform Example application (UDP or TCP)
 * @author     DCO
 * @date       Nov 2021
 *----------------------------------------------------------------------------
 *
 * @details      This application implements a UDP/TCP-oriented Vitis function.
 *
 * @deprecated   For the time being, we continue designing with the DEPRECATED
 *               directives because the new PRAGMAs do not work for us.
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup WarpTransformHLS
 * @addtogroup WarpTransformHLS
 * \{
 *****************************************************************************/
 
#include "../include/warp_transform.hpp"
#include "../include/xf_warp_transform_config.h"
#include "../include/warp_transform_network_library.hpp"
#include "../include/warp_transform_hw_common.hpp"
#include "../include/warp_transform_processing.hpp"

using hls::stream;


/*****************************************************************************
 * @brief   Main process of the WarpTransform Application 
 * directives.
 * @deprecated  This functions is using deprecated AXI stream interface 
 * @return Nothing.
 *****************************************************************************/
void warp_transform(

    ap_uint<32>                 *pi_rank,
    ap_uint<32>                 *pi_size,
    //------------------------------------------------------
    //-- SHELL / This / UDP/TCP Interfaces
    //------------------------------------------------------
    stream<NetworkWord>         &siSHL_This_Data,
    stream<NetworkWord>         &soTHIS_Shl_Data,
    stream<NetworkMetaStream>   &siNrc_meta,
    stream<NetworkMetaStream>   &soNrc_meta,
    ap_uint<32>                 *po_rx_ports
    
    #ifdef ENABLE_DDR
                                ,
    //------------------------------------------------------
    //-- SHELL / Role / Mem / Mp0 Interface
    //------------------------------------------------------
    //---- Read Path (MM2S) ------------
    // stream<DmCmd>               &soMemRdCmdP0,
    // stream<DmSts>               &siMemRdStsP0,
    // stream<Axis<MEMDW_512 > >   &siMemReadP0,
    //---- Write Path (S2MM) -----------
    stream<DmCmd>               &soMemWrCmdP0,
    stream<DmSts>               &siMemWrStsP0,
    stream<Axis<MEMDW_512> >    &soMemWriteP0,
    //------------------------------------------------------
    //-- SHELL / Role / Mem / Mp1 Interface
    //------------------------------------------------------    
    membus_t                    *lcl_mem0,
    membus_t                    *lcl_mem1
    #endif
    )
{
    

//-- DIRECTIVES FOR THE BLOCK ---------------------------------------------
#pragma HLS INTERFACE axis register both port=siSHL_This_Data
#pragma HLS INTERFACE axis register both port=soTHIS_Shl_Data

#pragma HLS INTERFACE axis register both port=siNrc_meta
#pragma HLS INTERFACE axis register both port=soNrc_meta

#pragma HLS INTERFACE ap_ovld register port=po_rx_ports name=poROL_NRC_Rx_ports
    
#if HLS_VERSION < 20211
#pragma HLS INTERFACE ap_stable register port=pi_rank name=piFMC_ROL_rank
#pragma HLS INTERFACE ap_stable register port=pi_size name=piFMC_ROL_size
#elif HLS_VERSION >= 20211
  #pragma HLS stable variable=pi_rank
  #pragma HLS stable variable=pi_size    
#else
    printf("ERROR: Invalid HLS_VERSION=%s\n", HLS_VERSION);
    exit(-1);
#endif
    
#ifdef ENABLE_DDR

// Bundling: SHELL / Role / Mem / Mp0 / Write Interface
#pragma HLS INTERFACE axis register both port=soMemWrCmdP0
#pragma HLS INTERFACE axis register both port=siMemWrStsP0
#pragma HLS INTERFACE axis register both port=soMemWriteP0

#if HLS_VERSION <= 20201
#pragma HLS DATA_PACK variable=soMemWrCmdP0 instance=soMemWrCmdP0
#pragma HLS DATA_PACK variable=siMemWrStsP0 instance=siMemWrStsP0    
#elif HLS_VERSION >= 20211
#pragma HLS aggregate variable=soMemWrCmdP0 compact=bit    
#pragma HLS aggregate variable=siMemWrStsP0 compact=bit  
#else
    printf("ERROR: Invalid HLS_VERSION=%s\n", HLS_VERSION);
    exit(-1);
#endif
    
const unsigned int ddr_mem_depth = TOTMEMDW_512;
const unsigned int ddr_latency = DDR_LATENCY;


// When max burst size is 1KB, with 512bit bus we get 16 burst transactions
// When max burst size is 4KB, with 512bit bus we get 64 burst transactions
const unsigned int max_axi_rw_burst_length = 64;
const unsigned int num_outstanding_transactions = 256;//16;

// Mapping LCL_MEM0 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem0 bundle=moMEM_Mp1\
  max_read_burst_length=max_axi_rw_burst_length  max_write_burst_length=max_axi_rw_burst_length offset=direct \
  num_read_outstanding=num_outstanding_transactions num_write_outstanding=num_outstanding_transactions latency=ddr_latency

// Mapping LCL_MEM1 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem1 bundle=moMEM_Mp1 \
  max_read_burst_length=max_axi_rw_burst_length  max_write_burst_length=max_axi_rw_burst_length offset=direct \
  num_read_outstanding=num_outstanding_transactions num_write_outstanding=num_outstanding_transactions latency=ddr_latency

#endif
 
 #pragma HLS DATAFLOW
 
  //-- LOCAL VARIABLES ------------------------------------------------------
  NetworkMetaStream  meta_tmp = NetworkMetaStream();
  static stream<NetworkWord>       sRxpToTxp_Data("sRxpToTxP_Data"); // FIXME: works even with no static
  static stream<NetworkMetaStream> sRxtoTx_Meta("sRxtoTx_Meta");
  static unsigned int processed_word_rx;
  static unsigned int processed_bytes_rx;
  static unsigned int processed_word_tx = 0;
  static stream<bool> sImageLoaded("sImageLoaded");
  static bool skip_read;
  static bool write_chunk_to_ddr_pending;
  static bool ready_to_accept_new_data;
  static bool signal_init;
  const int tot_transfers = TOT_TRANSFERS;
#ifdef ENABLE_DDR
    static stream<ap_uint<MEMDW_512>> img_in_axi_stream ("img_in_axi_stream");
    const unsigned int img_in_axi_stream_depth = TRANSFERS_PER_CHUNK; // the AXI burst size
    static stream<bool>               sMemBurstRx("sMemBurstRx");
    
#else
  const int img_in_axi_stream_depth = MIN_RX_LOOPS;
  const int img_out_axi_stream_depth = MIN_TX_LOOPS;
  static stream<ap_uint<INPUT_PTR_WIDTH>> img_in_axi_stream ("img_in_axi_stream");
  static stream<ap_uint<OUTPUT_PTR_WIDTH>> img_out_axi_stream ("img_out_axi_stream");
#endif
  static stream<NodeId>            sDstNode_sig("sDstNode_sig");


//-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS stream variable=sRxtoTx_Meta depth=tot_transfers
#pragma HLS reset variable=processed_word_rx
#pragma HLS reset variable=processed_word_tx
#pragma HLS reset variable=processed_bytes_rx
//#pragma HLS reset variable=image_loaded
#pragma HLS stream variable=sImageLoaded depth=1
#pragma HLS reset variable=skip_read
#pragma HLS reset variable=write_chunk_to_ddr_pending
//#pragma HLS stream variable=sWriteChunkToDdrPending depth=2
#pragma HLS reset variable=ready_to_accept_new_data
#pragma HLS reset variable=signal_init
#pragma HLS STREAM variable=sDstNode_sig     depth=1

#ifdef ENABLE_DDR
#pragma HLS stream variable=img_in_axi_stream depth=img_in_axi_stream_depth  
#pragma HLS stream variable=sProcessed_bytes_rx depth=img_in_axi_stream_depth
#else
#pragma HLS stream variable=img_in_axi_stream depth=img_in_axi_stream_depth
#pragma HLS stream variable=img_out_axi_stream depth=img_out_axi_stream_depth
#endif


  
 pPortAndDestionation(
        pi_rank, 
        pi_size, 
        sDstNode_sig, 
        po_rx_ports
        );
  
#ifdef ENABLE_DDR

 pRXPathNetToStream(
        siSHL_This_Data,
        siNrc_meta,
        sRxtoTx_Meta,
        img_in_axi_stream,
        sMemBurstRx
    );
 
 pRXPathStreamToDDR(
        img_in_axi_stream,
        sMemBurstRx,
        //---- P0 Write Path (S2MM) -----------
        soMemWrCmdP0,
        siMemWrStsP0,
        soMemWriteP0,
        //---- P1 Memory mapped ---------------
        //&processed_bytes_rx,
        sImageLoaded
    );
 
 
 
 #else // !ENABLE_DDR
  
 pRXPath(
        siSHL_This_Data,
        siNrc_meta,
        sRxtoTx_Meta,
        img_in_axi_stream,
        meta_tmp,
        &processed_word_rx,
        &processed_bytes_rx,
        sImageLoaded
        );
 
#endif // ENABLE_DDR

  pProcPath(
        sRxpToTxp_Data,
#ifdef ENABLE_DDR
        lcl_mem0,
        lcl_mem1,
#else
        img_in_axi_stream,
        img_out_axi_stream,
#endif
        sImageLoaded
        );

  pTXPath(
        soTHIS_Shl_Data,
        soNrc_meta,
        sRxpToTxp_Data,
        sRxtoTx_Meta,
        sDstNode_sig,
        &processed_word_tx,
        pi_rank,
        pi_size
        );
}


/*! \} */
