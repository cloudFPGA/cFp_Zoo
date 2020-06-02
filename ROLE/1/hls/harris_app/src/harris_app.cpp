/*****************************************************************************
 * @file       harris_app.cpp
 * @brief      The Role for a Harris Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID
 * @date       May 2020
 *----------------------------------------------------------------------------
 *
 * @details    : This application implements a set of UDP-oriented tests and
 *  functions which are embedded into the Flash of the cloudFPGA role.
 *
 * @deprecated   For the time being, we continue designing with the DEPRECATED
 *               directives because the new PRAGMAs do not work for us.
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup HarrisHLS
 * @addtogroup HarrisHLS
 * \{
 *****************************************************************************/

#include "../include/harris_app.hpp"

#include "../include/xf_harris_config.h"

//#include "../../../../../hlslib/include/hlslib/xilinx/Stream.h"
//#include "../../../../../hlslib/include/hlslib/xilinx/Simulation.h"

//using hlslib::Stream;
using hls::stream;

//#define Data_t ap_uint<INPUT_PTR_WIDTH>
//#define Data_t NetworkWord
#define Data_t ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0>

//stream<NetworkWord>       sRxpToTxp_Data("sRxpToTxP_Data");
//stream<NetworkMetaStream> sRxtoTx_Meta("sRxtoTx_Meta");

PacketFsmType enqueueFSM = WAIT_FOR_META;
PacketFsmType dequeueFSM = WAIT_FOR_STREAM_PAIR;
PacketFsmType HarrisFSM  = WAIT_FOR_META;

//unsigned int processed_word = 0;
//unsigned int image_loaded = 0;
unsigned int run_harris_once = 1;

//hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> > img_in_axi_stream("img_in_axi_stream");
//hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> > img_out_axi_stream("img_out_axi_stream");


/*****************************************************************************
 * @brief   Store a word from ethernet to local memory
 * @return Nothing.
 *****************************************************************************/
void storeWordToArray(uint64_t input, ap_uint<INPUT_PTR_WIDTH> img[IMG_PACKETS], unsigned int *processed_word, unsigned int *image_loaded)
{
  #pragma HLS INLINE
  
  img[*processed_word] = (ap_uint<INPUT_PTR_WIDTH>) input;
  printf("DEBUG in storeWordToArray: input = %u = 0x%16.16llX \n", input, input);
  printf("DEBUG in storeWordToArray: img[%u]= %u = 0x%16.16llX \n", *processed_word, 
  (uint64_t)img[*processed_word], (uint64_t)img[*processed_word]);
  if (*processed_word < IMG_PACKETS-1) {
    *processed_word++;
  }
  else {
    printf("DEBUG in storeWordToArray: WARNING - you've reached the max depth of img[%u]. Will put *processed_word = 0.\n", *processed_word);
    *processed_word = 0;
    *image_loaded = 1;
  }
}


/*****************************************************************************
 * @brief   Store a word from ethernet to a local AXI stream
 * @return Nothing.
 *****************************************************************************/
void storeWordToAxiStream(
  NetworkWord word, 
  //Stream<Data_t, IMG_PACKETS>                          &img_in_axi_stream,
  stream<Data_t>                                   &img_in_axi_stream,
  unsigned int *processed_word, 
  unsigned int *image_loaded)
{   
  #pragma HLS INLINE
  
  Data_t v;
  v.data = word.tdata;
  v.keep = word.tkeep;
  v.last = word.tlast;
  
  //Data_t v;
  //v = word.tdata;
  
  img_in_axi_stream.write(v);
  
  if (*processed_word < IMG_PACKETS-1) {
    (*processed_word)++;
  }
  else {
    printf("DEBUG in storeWordToAxiStream: WARNING - you've reached the max depth of img. Will put *processed_word = 0.\n");
    *processed_word = 0;
    *image_loaded = 1;
  }
  //printf("DEBUG in storeWordToAxiStream: input = %u = 0x%16.16llX , *processed_word = %u\n", (uint64_t)v.data, (uint64_t)v.data, *processed_word);
  //printf("DEBUG in storeWordToAxiStream: input = %u = 0x%16.16llX , *processed_word = %u, *image_loaded = %u\n", (uint64_t)v, (uint64_t)v, *processed_word, *image_loaded);
}





/*****************************************************************************
 * @brief Receive Path - From SHELL to THIS.
 *
 * @param[in]  piSHL_MmioEchoCtrl  configuration of the echo function.
 * @param[in]  siSHL_Data          data from SHELL.
 * @param[out] soEPt_Data          data to pEchoPassTrough.
 * @param[out] soESf_Data          data to pEchoStoreAndForward.
 *
 * @return Nothing.
 ******************************************************************************/
void pRXPath(
	stream<NetworkWord>                              &siSHL_This_Data,
        stream<NetworkMetaStream>                        &siNrc_meta,
	stream<NetworkWord>                              &sRxpToTxp_Data,
	stream<NetworkMetaStream>                        &sRxtoTx_Meta,
	//Stream<Data_t, IMG_PACKETS>                      &img_in_axi_stream,
	stream<Data_t>                                   &img_in_axi_stream,
        NetworkMetaStream                                meta_tmp,
	unsigned int                                     *processed_word, 
	unsigned int                                     *image_loaded
	    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    //#pragma HLS DATAFLOW interval=1
     #pragma  HLS INLINE
    //-- LOCAL VARIABLES ------------------------------------------------------
    UdpWord    udpWord;

  switch(enqueueFSM)
  {
    case WAIT_FOR_META: 
      printf("DEBUG in pRXPath: enqueueFSM - WAIT_FOR_META\n");
      if ( !siNrc_meta.empty() && !sRxtoTx_Meta.full() )
      {
        meta_tmp = siNrc_meta.read();
        meta_tmp.tlast = 1; //just to be sure...
        sRxtoTx_Meta.write(meta_tmp);
        enqueueFSM = PROCESSING_PACKET;
      }
      *processed_word = 0;
      *image_loaded = 0;
      break;

    case PROCESSING_PACKET:
      printf("DEBUG in pRXPath: enqueueFSM - PROCESSING_PACKET\n");
      //if ( !siSHL_This_Data.empty() && !sRxpToTxp_Data.full()  && !img_in_axi_stream.full() )
      if ( !siSHL_This_Data.empty() && !img_in_axi_stream.full() )
      {
        //-- Read incoming data chunk
        udpWord = siSHL_This_Data.read();
	////storeWordToArray(udpWord.tdata, img_inp);
	storeWordToAxiStream(udpWord, img_in_axi_stream, processed_word, image_loaded);
	//udpWord.tdata += 1;
	//udpWord.tdata += image_loaded;
        //sRxpToTxp_Data.write(udpWord);
	//sRxpToTxp_Data.write(img_in_axi_stream.read());
        if(udpWord.tlast == 1)
        {
          enqueueFSM = WAIT_FOR_META;
        }
      }
      break;
  }

 
}


/*****************************************************************************
 * @brief Receive Path - From SHELL to THIS.
 *
 * @param[in]  piSHL_MmioEchoCtrl  configuration of the echo function.
 * @param[in]  siSHL_Data          data from SHELL.
 * @param[out] soEPt_Data          data to pEchoPassTrough.
 * @param[out] soESf_Data          data to pEchoStoreAndForward.
 *
 * @return Nothing.
 ******************************************************************************/
void pProcPath(
	      stream<NetworkWord>                               &sRxpToTxp_Data,
	      //Stream<Data_t, IMG_PACKETS>                          &img_in_axi_stream,
              //Stream<Data_t, IMG_PACKETS>                          &img_out_axi_stream,
	      stream<Data_t>                          &img_in_axi_stream,
              stream<Data_t>                          &img_out_axi_stream,
	      unsigned int                                      *processed_word_tx, 
	      unsigned int                                      *image_loaded
	      )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    //#pragma HLS DATAFLOW interval=1
    #pragma  HLS INLINE
    //-- LOCAL VARIABLES ------------------------------------------------------
    Data_t oldWord;
    NetworkWord newWord;
    uint16_t Thresh = 442;
    float K = 0.04;
    uint16_t k = K * (1 << 16); // Convert to Q0.16 format
  
  
  switch(HarrisFSM)
  {
    case WAIT_FOR_META: 
      printf("DEBUG in pProcPath: WAIT_FOR_META\n");
      if ( (*image_loaded) == 1 )
      {
        HarrisFSM = PROCESSING_PACKET;
	*processed_word_tx = 0;
      }
      //if (!img_in_axi_stream.empty())
      //{
	//HarrisFSM = PROCESSING_PACKET;
      //}      
      break;

    case PROCESSING_PACKET:
      printf("DEBUG in pProcPath: PROCESSING_PACKET\n");
      if ( !img_in_axi_stream.empty() && !img_out_axi_stream.full() )
      {
	//if (img_in_axi_stream.full())
	//{
	  cornerHarrisAccelStream(img_in_axi_stream, img_out_axi_stream, WIDTH, HEIGHT, Thresh, k);
	//}
	//oldWord = img_in_axi_stream.read();
	//img_out_axi_stream.write(oldWord);
	//if (oldWord.last == 1)
	//if ( (!img_out_axi_stream.empty()) && ((*processed_word_tx)++ == IMG_PACKETS-1) )
	if ( !img_out_axi_stream.empty() )  
	{
	  HarrisFSM = HARRIS_RETURN_RESULTS;
	} 
      }
      break;
      
    case HARRIS_RETURN_RESULTS:
      printf("DEBUG in pProcPath: HARRIS_RETURN_RESULTS\n");
      if ( !img_out_axi_stream.empty() && !sRxpToTxp_Data.full() )
      {
	
	Data_t temp = img_out_axi_stream.read();
	if ( img_out_axi_stream.empty() ) 
	{
	  temp.last = 1;
	  *processed_word_tx = 0;
	  HarrisFSM = WAIT_FOR_META;
	}
	else
	{
	  temp.last = 0;
	  //(*processed_word_tx)++;
	}
	//TODO: find why Vitis kernel does not set keep and last by itself
	temp.keep = 255;
	newWord = NetworkWord(temp.data, temp.keep, temp.last); 
	/*
	if (*processed_word_tx < IMG_PACKETS - 1) {
	  //newWord = NetworkWord(img_out_axi_stream.read().data, 255, 0);
	  newWord = NetworkWord(img_out_axi_stream.read(), 255, 0);
	  
	  HarrisFSM = HARRIS_RETURN_RESULTS;
	}
	else {
	  printf("DEBUG in pProcPath: WARNING - you've reached the max depth of img[%u]. Will put *processed_word_tx = 0.\n", *processed_word_tx);
	  //newWord = NetworkWord(img_out_axi_stream.read().data, 255, 1);
	  newWord = NetworkWord(img_out_axi_stream.read(), 255, 1);
	  *processed_word_tx = 0;
	  HarrisFSM = WAIT_FOR_META;
	}*/
	/**/sRxpToTxp_Data.write(newWord);
      }
      break;
      
  } // end switch
 
}



/*****************************************************************************
 * @brief Transmit Path - From THIS to SHELL.
 *
 * @param[in]  piSHL_MmioEchoCtrl, configuration of the echo function.
 * @param[in]  siEPt_Data,         data from pEchoPassTrough.
 * @param[in]  siESf_Data,         data from pEchoStoreAndForward.
 * @param[out] soSHL_Data,         data to SHELL.
 *
 * @return Nothing.
 *****************************************************************************/
void pTXPath(
        stream<NetworkWord>         &soTHIS_Shl_Data,
        stream<NetworkMetaStream>   &soNrc_meta,
	stream<NetworkWord>         &sRxpToTxp_Data,
	stream<NetworkMetaStream>   &sRxtoTx_Meta,
        ap_uint<32>                 *pi_rank,
        ap_uint<32>                 *pi_size,
	unsigned int                *processed_word,
	unsigned int                *image_loaded
	    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    //#pragma HLS DATAFLOW interval=1
    #pragma  HLS INLINE
    //-- LOCAL VARIABLES ------------------------------------------------------
    UdpWord      udpWordTx;
    NetworkMeta  meta_in = NetworkMeta();
  
  switch(dequeueFSM)
  {
    case WAIT_FOR_STREAM_PAIR:
      printf("DEBUG in pTXPath: dequeueFSM=%d - WAIT_FOR_STREAM_PAIR, *processed_word=%u\n", dequeueFSM, *processed_word);
      //-- Forward incoming chunk to SHELL
      if (( !sRxpToTxp_Data.empty() && !sRxtoTx_Meta.empty() 
          && !soTHIS_Shl_Data.full() &&  !soNrc_meta.full() )) 
      {
        udpWordTx = sRxpToTxp_Data.read();
        soTHIS_Shl_Data.write(udpWordTx);

        meta_in = sRxtoTx_Meta.read().tdata;
        NetworkMetaStream meta_out_stream = NetworkMetaStream();
        meta_out_stream.tlast = 1;
        meta_out_stream.tkeep = 0xFF; //just to be sure

        //printf("rank: %d; size: %d; \n", (int) *pi_rank, (int) *pi_size);
        meta_out_stream.tdata.dst_rank = (*pi_rank + 1) % *pi_size;
        //printf("meat_out.dst_rank: %d\n", (int) meta_out_stream.tdata.dst_rank);

        meta_out_stream.tdata.dst_port = DEFAULT_TX_PORT;
        meta_out_stream.tdata.src_rank = (NodeId) *pi_rank;
        meta_out_stream.tdata.src_port = DEFAULT_RX_PORT;
        soNrc_meta.write(meta_out_stream);

        if(udpWordTx.tlast != 1)
        {
          dequeueFSM = PROCESSING_PACKET;
        }
      }
      break;

    case PROCESSING_PACKET: 
      printf("DEBUG in pTXPath: dequeueFSM=%d - PROCESSING_PACKET\n", dequeueFSM);
      if( !sRxpToTxp_Data.empty() && !soTHIS_Shl_Data.full())
      {
        udpWordTx = sRxpToTxp_Data.read();

        soTHIS_Shl_Data.write(udpWordTx);

        if(udpWordTx.tlast == 1)
        {
          dequeueFSM = WAIT_FOR_STREAM_PAIR;
        }

      }
      break;
  }
}


/*****************************************************************************
 * @brief   Main process of the Harris Application 
 * directives.
 * @deprecated  This functions is using deprecated AXI stream interface 
 * @return Nothing.
 *****************************************************************************/
void harris_app(

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
  static unsigned int processed_word_tx;
  static unsigned int image_loaded;
  const int img_packets = IMG_PACKETS;
  
  //static stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >  img_in_axi_stream ("img_in_axi_stream" );
  //static stream<ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0> > img_out_axi_stream("img_out_axi_stream");
  
  static stream<Data_t> img_in_axi_stream ("img_in_axi_stream" );
  static stream<Data_t> img_out_axi_stream("img_out_axi_stream"); 
  
  //static Stream<Data_t, IMG_PACKETS>  img_in_axi_stream ("img_in_axi_stream");
  //static Stream<Data_t, IMG_PACKETS>  img_out_axi_stream ("img_out_axi_stream");
  
  //ap_uint<INPUT_PTR_WIDTH> img_inp[IMGSIZE];
  //ap_uint<OUTPUT_PTR_WIDTH> img_out[IMGSIZE];
  *po_rx_ports = 0x1; //currently work only with default ports...

  
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS DATAFLOW 
//#pragma HLS STREAM variable=sRxpToTxp_Data depth=1500 
//#pragma HLS STREAM variable=sRxtoTx_Meta depth=1500 
#pragma HLS reset variable=enqueueFSM
#pragma HLS reset variable=dequeueFSM
#pragma HLS reset variable=HarrisFSM
#pragma HLS reset variable=processed_word_rx
#pragma HLS reset variable=processed_word_tx
#pragma HLS reset variable=image_loaded
#pragma HLS stream variable=img_in_axi_stream depth=img_packets
#pragma HLS stream variable=img_out_axi_stream depth=img_packets
  

  /*
  // Dataflow functions running in parallel
  HLSLIB_DATAFLOW_INIT();
  
  HLSLIB_DATAFLOW_FUNCTION(pRXPath, 
			   siSHL_This_Data,
			   siNrc_meta,
			   sRxpToTxp_Data,
			   sRxtoTx_Meta,
			   img_in_axi_stream,
			   meta_tmp,
			   &processed_word_rx,
			   &image_loaded);
  
  HLSLIB_DATAFLOW_FUNCTION(pProcPath,
			   sRxpToTxp_Data,
		           img_in_axi_stream,
		           img_out_axi_stream,
		           &processed_word_tx,
		           &image_loaded); 

  HLSLIB_DATAFLOW_FUNCTION(pTXPath,
			   soTHIS_Shl_Data,
			   soNrc_meta,
			   sRxpToTxp_Data,
			   sRxtoTx_Meta,
			   pi_rank,
			   pi_size,
			   &processed_word_tx,
			   &image_loaded);

  HLSLIB_DATAFLOW_FINALIZE();
  */
  
 pRXPath(
	siSHL_This_Data,
        siNrc_meta,
	sRxpToTxp_Data,
	sRxtoTx_Meta,
	img_in_axi_stream,
        meta_tmp,	 
        &processed_word_rx,
	&image_loaded);
  
  
  /* --------------------------------------------------------------------------------------------
  // spare placeholder of Harris IP with array I/F
  if (image_loaded == 1) {
    printf("DEBUG in harris_app: image_loaded => cornerHarrisAccelArray(), processed_word=%u\n", 
processed_word);
    if (run_harris_once == 1) {
      cornerHarrisAccelArray(img_inp, img_out, WIDTH, HEIGHT, Thresh, k);
      run_harris_once = 0;
    }
    if (processed_word < IMG_PACKETS - 1) {
      newWord = NetworkWord(img_out[processed_word], 255, 0);
      newWord = NetworkWord(img_out_axi_stream.read().data, 255, 0);
      processed_word++;
    }
    else {
      printf("DEBUG in harris_app: WARNING - you've reached the max depth of img[%u]. Will put processed_word = 0.\n", processed_word);
      newWord = NetworkWord(img_out[processed_word], 255, 1);
      newWord = NetworkWord(img_out_axi_stream.read().data, 255, 1);
      processed_word = 0;
      image_loaded = 0; // force reset
    }
    sRxpToTxp_Data.write(newWord);
  }
   -------------------------------------------------------------------------------------------- */
  
  
  pProcPath(sRxpToTxp_Data,
	    img_in_axi_stream,
	    img_out_axi_stream,
	    &processed_word_tx,
	    &image_loaded
  );
  


  
  
  
  /*
    // spare placeholder of Harris IP with stream I/F
  if (image_loaded == 1) {
    printf("DEBUG in harris_app: image_loaded => cornerHarrisAccelStream(), processed_word=%u\n", 
processed_word);
    if (run_harris_once == 1) {
      cornerHarrisAccelStream(img_in_axi_stream, img_out_axi_stream, WIDTH, HEIGHT, Thresh, k);
      run_harris_once = 0;
    }
    if (processed_word < IMG_PACKETS - 1) {
      newWord = NetworkWord(img_out_axi_stream.read().data, 255, 0);
      processed_word++;
    }
    else {
      printf("DEBUG in harris_app: WARNING - you've reached the max depth of img[%u]. Will put processed_word = 0.\n", processed_word);
      newWord = NetworkWord(img_out_axi_stream.read().data, 255, 1);
      processed_word = 0;
      image_loaded = 0; // force reset
    }    
    sRxpToTxp_Data.write(newWord);
  }
  */
  
  
  pTXPath(
        soTHIS_Shl_Data,
        soNrc_meta,
	sRxpToTxp_Data,
	sRxtoTx_Meta,
        pi_rank,
        pi_size,
	&processed_word_tx,
	&image_loaded
	);
  
  
  
  


}


/*! \} */