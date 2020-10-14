/*****************************************************************************
 * @file       mceuropeanengine.cpp
 * @brief      The Role for a MCEuropeanEngine Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID
 * @date       October 2020
 *----------------------------------------------------------------------------
 *
 * @details      This application implements a UDP/TCP-oriented Vitis function.
 *
 * @deprecated   For the time being, we continue designing with the DEPRECATED
 *               directives because the new PRAGMAs do not work for us.
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup MCEuropeanEngineHLS
 * @addtogroup MCEuropeanEngineHLS
 * \{
 *****************************************************************************/

#include "../include/mceuropeanengine.hpp"
#include "../../../../../HOST/quantitative_finance/mceuropeanengine/languages/cplusplus/include/config.h"

#ifdef USE_HLSLIB_DATAFLOW
#include "../../../../../hlslib/include/hlslib/xilinx/Stream.h"
#include "../../../../../hlslib/include/hlslib/xilinx/Simulation.h"
#endif

#ifdef USE_HLSLIB_STREAM
using hlslib::Stream;
#endif
using hls::stream;

PacketFsmType enqueueFSM = WAIT_FOR_META;
PacketFsmType dequeueFSM = WAIT_FOR_STREAM_PAIR;
PacketFsmType MCEuropeanEngineFSM  = WAIT_FOR_META;


/*****************************************************************************
 * @brief   Store a word from ethernet to a local AXI stream
 * @return Nothing.
 *****************************************************************************/
void storeWordToStruct(
  NetworkWord word,
  varin *instruct,
  unsigned int *processed_word_rx,
  unsigned int *processed_bytes_rx,
  unsigned int *struct_loaded)
{   
  //#pragma HLS INLINE
  const unsigned int loop_cnt = (BITS_PER_10GBITETHRNET_AXI_PACKET/INPUT_PTR_WIDTH);
  const unsigned int bytes_per_loop = (BYTES_PER_10GBITETHRNET_AXI_PACKET/loop_cnt);
  unsigned int bytes_with_keep = 0;
  //v = word.tdata;
  for (unsigned int i=0; i<loop_cnt; i++) {
    //#pragma HLS PIPELINE
    //#pragma HLS UNROLL factor=loop_cnt
    printf("DEBUG: Checking: word.tkeep=%u >> %u = %u\n", word.tkeep.to_int(), i, (word.tkeep.to_int() >> i));
    if ((word.tkeep >> i) == 0) {
      printf("WARNING: value with tkeep=0 at i=%u\n", i);
      continue; 
    }
    intToFloatUnion intToFloat;
    intToFloat.i  = (DtUsedInt)(word.tdata >> i*loop_cnt);
    switch((*processed_word_rx)++)
      {
	case 0:
	  instruct->loop_nm = intToFloat.i;
	  printf("DEBUG instruct->loop_nm = %u\n", instruct->loop_nm);
	  break;
	case 1:
	  instruct->seed = intToFloat.i;
	  printf("DEBUG instruct->seed = %u\n", instruct->seed);
	  break;  
	case 2:
	  instruct->underlying = intToFloat.f;
	  printf("DEBUG instruct->underlying = %f\n", instruct->underlying);
	  break;
	case 3:
	  instruct->volatility = intToFloat.f;
	  printf("DEBUG instruct->volatility = %f\n", instruct->volatility);
	  break;
	case 4:
	  instruct->dividendYield = intToFloat.f;
	  printf("DEBUG instruct->dividendYield = %f\n", instruct->dividendYield);
	  break;
	case 5:
	  instruct->riskFreeRate = intToFloat.f;
	  printf("DEBUG instruct->riskFreeRate = %f\n", instruct->riskFreeRate);
	  break;
	case 6:
	  instruct->timeLength = intToFloat.f;
	  printf("DEBUG instruct->timeLength = %f\n", instruct->timeLength);
	  break;  
	case 7:
	  instruct->strike = intToFloat.f;
	  printf("DEBUG instruct->strike = %f\n", instruct->strike);
	  break;
	case 8:
	  instruct->optionType = intToFloat.i;
	  printf("DEBUG instruct->optionType = %u\n", instruct->optionType);
	  break;
	case 9:
	  instruct->requiredTolerance = intToFloat.f;
	  printf("DEBUG instruct->requiredTolerance = %f\n", instruct->requiredTolerance);
	  break;
	case 10:
	  instruct->requiredSamples = intToFloat.i;
	  printf("DEBUG instruct->requiredSamples = %u\n", instruct->requiredSamples);
	  break;
	case 11:
	  instruct->timeSteps = intToFloat.i;
	  printf("DEBUG instruct->timeSteps = %u\n", instruct->timeSteps);
	  break;  
	case 12:
	  instruct->maxSamples = intToFloat.i;
	  printf("DEBUG instruct->maxSamples = %u\n", instruct->maxSamples);
	  break;
      }
    bytes_with_keep += bytes_per_loop;
  }

  if (*processed_bytes_rx < INSIZE-BYTES_PER_10GBITETHRNET_AXI_PACKET) {
    (*processed_bytes_rx) += bytes_with_keep;
  }
  else {
    printf("DEBUG in storeWordToStruct: WARNING - you've reached the max depth of struct. Will put *processed_bytes_rx = 0.\n");
    *processed_bytes_rx = 0;
    *struct_loaded = 1;
  }
}



/*****************************************************************************
 * @brief Receive Path - From SHELL to THIS.
 *
 * @param[in]  siSHL_This_Data
 * @param[in]  siNrc_meta
 * @param[out] sRxtoTx_Meta
 * @param[out] img_in_axi_stream
 * @param[out] meta_tmp
 * @param[out] processed_word
 * @param[out] struct_loaded
 *
 * @return Nothing.
 ******************************************************************************/
void pRXPath(
	stream<NetworkWord>                              &siSHL_This_Data,
        stream<NetworkMetaStream>                        &siNrc_meta,
	stream<NetworkMetaStream>                        &sRxtoTx_Meta,
	varin                                            *instruct,
	NetworkMetaStream                                meta_tmp,
	unsigned int                                     *processed_word_rx,
	unsigned int                                     *processed_bytes_rx,
	unsigned int                                     *struct_loaded
	    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    //#pragma HLS DATAFLOW interval=1
     #pragma  HLS INLINE
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord    netWord;

  switch(enqueueFSM)
  {
    case WAIT_FOR_META: 
      printf("DEBUG in pRXPath: enqueueFSM - WAIT_FOR_META, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
	     *processed_word_rx, *processed_bytes_rx);
      if ( !siNrc_meta.empty() && !sRxtoTx_Meta.full() )
      {
        meta_tmp = siNrc_meta.read();
        meta_tmp.tlast = 1; //just to be sure...
        sRxtoTx_Meta.write(meta_tmp);
        enqueueFSM = PROCESSING_PACKET;
      }
      *struct_loaded = 0;
      break;

    case PROCESSING_PACKET:
      printf("DEBUG in pRXPath: enqueueFSM - PROCESSING_PACKET, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
	     *processed_word_rx, *processed_bytes_rx);
      if ( !siSHL_This_Data.empty() )
      {
        //-- Read incoming data chunk
        netWord = siSHL_This_Data.read();
	storeWordToStruct(netWord, instruct, processed_word_rx, processed_bytes_rx, 
			     struct_loaded);
        if(netWord.tlast == 1)
        {
          enqueueFSM = WAIT_FOR_META;
        }
      }
      break;
  }

 
}


/*****************************************************************************
 * @brief Processing Path - Main processing FSM for Vitis kernels.
 *
 * @param[out] sRxpToTxp_Data
 * @param[in]  img_in_axi_stream
 * @param[in]  img_out_axi_stream
 * @param[out] processed_word_rx
 * @param[in]  struct_loaded
 *
 * @return Nothing.
 ******************************************************************************/
void pProcPath(
	      stream<NetworkWord>                    &sRxpToTxp_Data,
	      varin                                  *instruct,
	      DtUsed                                 *out,
	      unsigned int                           *processed_word_rx,
	      unsigned int                           *processed_bytes_rx, 
	      unsigned int                           *struct_loaded
	      )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    //#pragma HLS DATAFLOW interval=1
    #pragma  HLS INLINE
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord newWord;
    uint16_t Thresh = 442;
    float K = 0.04;
    uint16_t k = K * (1 << 16); // Convert to Q0.16 format
  
  
  switch(MCEuropeanEngineFSM)
  {
    case WAIT_FOR_META: 
      printf("DEBUG in pProcPath: WAIT_FOR_META\n");
      if ( (*struct_loaded) == 1 )
      {
        MCEuropeanEngineFSM = PROCESSING_PACKET;
	*processed_word_rx = 0;
	*processed_bytes_rx = 0;
      }
      break;

    case PROCESSING_PACKET:
      printf("DEBUG in pProcPath: PROCESSING_PACKET\n");
      //if ( !img_in_axi_stream.empty() && !img_out_axi_stream.full() )
      {
	kernel_mc(instruct->loop_nm,
                           instruct->seed,
                           instruct->underlying,
                           instruct->volatility,
                           instruct->dividendYield,
                           instruct->riskFreeRate, // model parameter
                           instruct->timeLength,
                           instruct->strike,
                           instruct->optionType, // option parameter
                           out,
                           instruct->requiredTolerance,
                           instruct->requiredSamples,
                           instruct->timeSteps,
                           instruct->maxSamples);
	MCEuropeanEngineFSM = MCEUROPEANENGINE_RETURN_RESULTS;
      }
      break;
      
    case MCEUROPEANENGINE_RETURN_RESULTS:
      printf("DEBUG in pProcPath: MCEUROPEANENGINE_RETURN_RESULTS\n");
      if ( !sRxpToTxp_Data.full() )
      {
	
	//Data_t_out temp = img_out_axi_stream.read();
	bool last;
	//if ( img_out_axi_stream.empty() ) 
	{
	  last = 1;
	  MCEuropeanEngineFSM = WAIT_FOR_META;
	}
	//else
	//{
	  last = 0;
	//}
	//TODO: find why Vitis kernel does not set keep and last by itself
	unsigned int keep = 255;
	newWord = NetworkWord(1, keep, last); 
	sRxpToTxp_Data.write(newWord);
      }
      break;
      
  } // end switch
 
}



/*****************************************************************************
 * @brief Transmit Path - From THIS to SHELL.
 *
 * @param[out] soTHIS_Shl_Data
 * @param[out] soNrc_meta
 * @param[in]  sRxpToTxp_Data
 * @param[in]  sRxtoTx_Meta
 * @param[in]  pi_rank
 * @param[in]  pi_size
 *
 * @return Nothing.
 *****************************************************************************/
void pTXPath(
        stream<NetworkWord>         &soTHIS_Shl_Data,
        stream<NetworkMetaStream>   &soNrc_meta,
	stream<NetworkWord>         &sRxpToTxp_Data,
	stream<NetworkMetaStream>   &sRxtoTx_Meta,
        unsigned int                *processed_word_tx, 
        ap_uint<32>                 *pi_rank,
        ap_uint<32>                 *pi_size
	    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    //#pragma HLS DATAFLOW interval=1
    #pragma  HLS INLINE
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord      netWordTx;
    NetworkMeta  meta_in = NetworkMeta();
  
  switch(dequeueFSM)
  {
    case WAIT_FOR_STREAM_PAIR:
      printf("DEBUG in pTXPath: dequeueFSM=%d - WAIT_FOR_STREAM_PAIR, *processed_word_tx=%u\n", 
	     dequeueFSM, *processed_word_tx);
      //-- Forward incoming chunk to SHELL
      *processed_word_tx = 0;
      
      /*
      printf("!sRxpToTxp_Data.empty()=%d\n", !sRxpToTxp_Data.empty());
      printf("!sRxtoTx_Meta.empty()=%d\n", !sRxtoTx_Meta.empty());
      printf("!soTHIS_Shl_Data.full()=%d\n", !soTHIS_Shl_Data.full());
      printf("!soNrc_meta.full()=%d\n", !soNrc_meta.full());
      */
      
      if (( !sRxpToTxp_Data.empty() && !sRxtoTx_Meta.empty() 
          && !soTHIS_Shl_Data.full() &&  !soNrc_meta.full() )) 
      {
        netWordTx = sRxpToTxp_Data.read();

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

        meta_out_stream.tdata.dst_rank = (*pi_rank + 1) % *pi_size;
        //meta_out_stream.tdata.dst_port = DEFAULT_TX_PORT;
        meta_out_stream.tdata.src_rank = (NodeId) *pi_rank;
        //meta_out_stream.tdata.src_port = DEFAULT_RX_PORT;
        //printf("rank: %d; size: %d; \n", (int) *pi_rank, (int) *pi_size);
        //printf("meat_out.dst_rank: %d\n", (int) meta_out_stream.tdata.dst_rank);
        meta_out_stream.tdata.dst_port = meta_in.src_port;
        meta_out_stream.tdata.src_port = meta_in.dst_port;
	
	
	//meta_out_stream.tdata.len = meta_in.len; 
        soNrc_meta.write(meta_out_stream);

	(*processed_word_tx)++;
	
        if(netWordTx.tlast != 1)
        {
          dequeueFSM = PROCESSING_PACKET;
        }
      }
      break;

    case PROCESSING_PACKET: 
      printf("DEBUG in pTXPath: dequeueFSM=%d - PROCESSING_PACKET, *processed_word_tx=%u\n", 
	     dequeueFSM, *processed_word_tx);
      if( !sRxpToTxp_Data.empty() && !soTHIS_Shl_Data.full())
      {
        netWordTx = sRxpToTxp_Data.read();

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
 * @brief   Main process of the MCEuropeanEngine Application 
 * directives.
 * @deprecated  This functions is using deprecated AXI stream interface 
 * @return Nothing.
 *****************************************************************************/
void mceuropeanengine(

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


  //-- LOCAL VARIABLES ------------------------------------------------------
  NetworkMetaStream  meta_tmp = NetworkMetaStream();
  static stream<NetworkWord>       sRxpToTxp_Data("sRxpToTxP_Data"); // FIXME: works even with no static
  static stream<NetworkMetaStream> sRxtoTx_Meta("sRxtoTx_Meta");
  static unsigned int processed_word_rx;
  static unsigned int processed_bytes_rx;
  static unsigned int processed_word_tx;
  static unsigned int struct_loaded;
  const int img_in_axi_stream_depth = MIN_RX_LOOPS;
  const int img_out_axi_stream_depth = MIN_TX_LOOPS;
  const int tot_transfers = TOT_TRANSFERS;
  varin instruct;
  DtUsed out[OUTDEP];
  *po_rx_ports = 0x1; //currently work only with default ports...

  
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS DATAFLOW 
#pragma HLS stream variable=sRxtoTx_Meta depth=tot_transfers 
#pragma HLS reset variable=enqueueFSM
#pragma HLS reset variable=dequeueFSM
#pragma HLS reset variable=MCEuropeanEngineFSM
#pragma HLS reset variable=processed_word_rx
#pragma HLS reset variable=processed_word_tx
#pragma HLS reset variable=struct_loaded
#pragma HLS stream variable=img_in_axi_stream depth=img_in_axi_stream_depth
#pragma HLS stream variable=img_out_axi_stream depth=img_out_axi_stream_depth
  

#ifdef USE_HLSLIB_DATAFLOW
  /*! @copybrief mceuropeanengine()
   *  MCEuropeanEngineFSM is enabled with hlslib support
   */
  /*! @copydoc mceuropeanengine()
   * Use this snippet to early check for C++ errors related to dataflow and bounded streams (empty 
   * and full) during simulation. It can also be both synthesized and used in co-simulation.
   * Practically we use hlslib when we want to run simulation as close as possible to the HW, by 
   * executing all functions of dataflow in thread-safe parallel executions, i.e the function 
   * HLSLIB_DATAFLOW_FINALIZE() acts as a barrier for the threads spawned to serve every function 
   * called in HLSLIB_DATAFLOW_FUNCTION(func, args...).
   */
   /*! @copydetails mceuropeanengine()
   * hlslib is a collection of C++ headers, CMake files, and examples, aimed at improving the 
   * quality of life of HLS developers. More info at: https://github.com/definelicht/hlslib
   */
  // Dataflow functions running in parallel
  HLSLIB_DATAFLOW_INIT();
  
  HLSLIB_DATAFLOW_FUNCTION(pRXPath, 
			   siSHL_This_Data,
			   siNrc_meta,
			   sRxtoTx_Meta,
			   img_in_axi_stream,
			   meta_tmp,
			   &processed_word_rx,
			   &processed_bytes_rx,
			   &struct_loaded);
  
  HLSLIB_DATAFLOW_FUNCTION(pProcPath,
			   sRxpToTxp_Data,
		           img_in_axi_stream,
		           img_out_axi_stream,
		           &processed_word_rx,
			   &processed_bytes_rx,
		           &struct_loaded); 

  HLSLIB_DATAFLOW_FUNCTION(pTXPath,
			   soTHIS_Shl_Data,
			   soNrc_meta,
			   sRxpToTxp_Data,
			   sRxtoTx_Meta,
			   &processed_word_tx,
			   pi_rank,
			   pi_size);

  HLSLIB_DATAFLOW_FINALIZE();
  
#else // !USE_HLSLIB_DATAFLOW
 pRXPath(
	siSHL_This_Data,
        siNrc_meta,
	sRxtoTx_Meta,
	&instruct,
        meta_tmp, 
        &processed_word_rx,
	&processed_bytes_rx,
	&struct_loaded);
  
  
  pProcPath(sRxpToTxp_Data,
	    &instruct,
	    out,
	    &processed_word_rx,
	    &processed_bytes_rx,
	    &struct_loaded);  
 
  pTXPath(
        soTHIS_Shl_Data,
        soNrc_meta,
	sRxpToTxp_Data,
	sRxtoTx_Meta,
        &processed_word_tx,
        pi_rank,
        pi_size);
#endif // USE_HLSLIB_DATAFLOW
}


/*! \} */