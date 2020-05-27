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


stream<NetworkWord>       sRxpToTxp_Data("sRxpToTxP_Data");
stream<NetworkMetaStream> sRxtoTx_Meta("sRxtoTx_Meta");

PacketFsmType enqueueFSM = WAIT_FOR_META;
PacketFsmType dequeueFSM = WAIT_FOR_STREAM_PAIR;
PacketFsmType HarrisFSM  = WAIT_FOR_META;

unsigned int processed_word = 0;
unsigned int image_loaded = 0;
unsigned int run_harris_once = 1;

hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> > img_in_axi_stream("img_in_axi_stream");
hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> > img_out_axi_stream("img_out_axi_stream");


/*****************************************************************************
 * @brief   Store a word from ethernet to local memory
 * @return Nothing.
 *****************************************************************************/
void storeWordToArray(uint64_t input, ap_uint<INPUT_PTR_WIDTH> img[IMGSIZE])
{
    img[processed_word] = (ap_uint<INPUT_PTR_WIDTH>) input;
    printf("DEBUG in storeWordToArray: input = %u = 0x%16.16llX \n", input, input);
    printf("DEBUG in storeWordToArray: img[%u]= %u = 0x%16.16llX \n", processed_word, 
(uint64_t)img[processed_word], (uint64_t)img[processed_word]);
    if (processed_word < IMG_PACKETS-1) {
      processed_word++;
    }
    else {
      printf("DEBUG in storeWordToArray: WARNING - you've reached the max depth of img[%u]. Will put processed_word = 0.\n", processed_word);
      processed_word = 0;
      image_loaded = 1;
    }
}


/*****************************************************************************
 * @brief   Store a word from ethernet to a local AXI stream
 * @return Nothing.
 *****************************************************************************/
void storeWordToAxiStream(NetworkWord word, hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >& img_in_axi_stream)
{
    
  ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> v;
  v.data = word.tdata;
  v.keep = word.tkeep;
  v.last = word.tlast;
  img_in_axi_stream.write(v);
  printf("DEBUG in storeWordToAxiStream: input = %u = 0x%16.16llX \n", (uint64_t)v.data, (uint64_t)v.data);
  
  if (processed_word < IMG_PACKETS-1) {
    processed_word++;
  }
  else {
    printf("DEBUG in storeWordToAxiStream: WARNING - you've reached the max depth of img. Will put processed_word = 0.\n");
    processed_word = 0;
    image_loaded = 1;
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


  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS DATAFLOW interval=1
//#pragma HLS STREAM variable=sRxpToTxp_Data depth=1500 
//#pragma HLS STREAM variable=sRxtoTx_Meta depth=1500 
#pragma HLS reset variable=enqueueFSM
#pragma HLS reset variable=dequeueFSM
#pragma HLS reset variable=HarrisFSM


  uint16_t Thresh = 442;
  float K = 0.04;
  uint16_t k = K * (1 << 16); // Convert to Q0.16 format
  //ap_uint<INPUT_PTR_WIDTH> img_inp[IMGSIZE];
  //ap_uint<OUTPUT_PTR_WIDTH> img_out[IMGSIZE];
  
  const int img_packets = IMG_PACKETS;
#pragma HLS stream variable=img_in_axi_stream depth=img_packets
#pragma HLS stream variable=img_out_axi_stream depth=img_packets
  
  *po_rx_ports = 0x1; //currently work only with default ports...

  //-- LOCAL VARIABLES ------------------------------------------------------
  NetworkWord udpWord;
  NetworkWord udpWordTx;
  NetworkWord newWord;
  NetworkMetaStream  meta_tmp = NetworkMetaStream();
  NetworkMeta  meta_in = NetworkMeta();


  switch(enqueueFSM)
  {
    case WAIT_FOR_META: 
      if ( !siNrc_meta.empty() && !sRxtoTx_Meta.full() )
      {
        meta_tmp = siNrc_meta.read();
        meta_tmp.tlast = 1; //just to be sure...
        sRxtoTx_Meta.write(meta_tmp);
        enqueueFSM = PROCESSING_PACKET;
      }
      break;

    case PROCESSING_PACKET:
      if ( !siSHL_This_Data.empty() && !sRxpToTxp_Data.full() )
      {
        //-- Read incoming data chunk
        udpWord = siSHL_This_Data.read();
	printf("DEBUG in harris_app: enqueueFSM - PROCESSING_PACKET\n");
	////storeWordToArray(udpWord.tdata, img_inp);
	storeWordToAxiStream(udpWord, img_in_axi_stream);
	udpWord.tdata += 1;
	udpWord.tdata += image_loaded;
        sRxpToTxp_Data.write(udpWord);
        if(udpWord.tlast == 1)
        {
          enqueueFSM = WAIT_FOR_META;
        }
      }
      break;
  }

 
  
  
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
  
  
  switch(HarrisFSM)
  {
    case WAIT_FOR_META: 
      printf("DEBUG in HarrisFSM: WAIT_FOR_META\n");
      if ( image_loaded == 1 )
      {
        HarrisFSM = PROCESSING_PACKET;
      }
      break;

    case PROCESSING_PACKET:
      printf("DEBUG in HarrisFSM: PROCESSING_PACKET\n");
      cornerHarrisAccelStream(img_in_axi_stream, img_out_axi_stream, WIDTH, HEIGHT, Thresh, k);
      HarrisFSM = HARRIS_RETURN_RESULTS;
      break;
      
    case HARRIS_RETURN_RESULTS:
      printf("DEBUG in HarrisFSM: HARRIS_RETURN_RESULTS\n");
      if (!img_out_axi_stream.empty()) {
	if (processed_word < IMG_PACKETS - 1) {
	  newWord = NetworkWord(img_out_axi_stream.read().data, 255, 0);
	  processed_word++;
	  HarrisFSM = HARRIS_RETURN_RESULTS;
	}
	else {
	  printf("DEBUG in harris_app: WARNING - you've reached the max depth of img[%u]. Will put processed_word = 0.\n", processed_word);
	  newWord = NetworkWord(img_out_axi_stream.read().data, 255, 1);
	  processed_word = 0;
	  image_loaded = 0; // force reset
	  HarrisFSM = WAIT_FOR_META;
	}
	//sRxpToTxp_Data.write(newWord);
      }
      break;
      
  }

  
  
  
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
  
  
  
  
  
  
  
  
  switch(dequeueFSM)
  {
    case WAIT_FOR_STREAM_PAIR:
      printf("DEBUG in harris_app: dequeueFSM=%d - WAIT_FOR_STREAM_PAIR, processed_word=%u\n", dequeueFSM, processed_word);
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
      if( !sRxpToTxp_Data.empty() && !soTHIS_Shl_Data.full())
      {
        udpWordTx = sRxpToTxp_Data.read();

        soTHIS_Shl_Data.write(udpWordTx);
	printf("DEBUG in harris_app: dequeueFSM=%d - PROCESSING_PACKET\n", dequeueFSM);

        if(udpWordTx.tlast == 1)
        {
          dequeueFSM = WAIT_FOR_STREAM_PAIR;
        }

      }
      break;
  }

}


/*! \} */