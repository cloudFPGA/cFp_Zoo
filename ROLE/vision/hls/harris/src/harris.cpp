/*****************************************************************************
 * @file       harris.cpp
 * @brief      The Role for a Harris Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID
 * @date       May 2020
 *----------------------------------------------------------------------------
 *
 * @details      This application implements a UDP/TCP-oriented Vitis function.
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
 
#include "../include/harris.hpp"
#include "../include/xf_harris_config.h"

#ifdef USE_HLSLIB_DATAFLOW
#include "../../../../../hlslib/include/hlslib/xilinx/Stream.h"
#include "../../../../../hlslib/include/hlslib/xilinx/Simulation.h"
#endif
 
#ifdef USE_HLSLIB_STREAM
using hlslib::Stream;
#endif
using hls::stream;

PacketFsmType  enqueueFSM  = WAIT_FOR_META;
PacketFsmType  dequeueFSM  = WAIT_FOR_STREAM_PAIR;
PacketFsmType  HarrisFSM   = WAIT_FOR_META;
fsmStateDDRdef fsmStateDDR = FSM_IDLE; //FSM_WR_PAT_CMD;


/*****************************************************************************
 * @brief   Store a net word to local memory
 * @return Nothing.
 *****************************************************************************/
void storeWordToArray(uint64_t input, ap_uint<INPUT_PTR_WIDTH> img[IMG_PACKETS], 
		      unsigned int *processed_word, unsigned int *image_loaded)
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
 * @brief   Store a net word to a local AXI stream
 * @return Nothing.
 *****************************************************************************/
void storeWordToAxiStream(
  NetworkWord word,
  #ifdef USE_HLSLIB_STREAM
  Stream<Data_t_in, MIN_RX_LOOPS>   &img_in_axi_stream,
  #else
  stream<Data_t_in>                 &img_in_axi_stream,
  #endif
  unsigned int  *processed_word_rx,
  unsigned int  *processed_bytes_rx,
  bool          *image_loaded)
{   
  #pragma HLS INLINE
  Data_t_in v;
  const unsigned int loop_cnt = (BITS_PER_10GBITETHRNET_AXI_PACKET/INPUT_PTR_WIDTH);
  const unsigned int bytes_per_loop = (BYTES_PER_10GBITETHRNET_AXI_PACKET/loop_cnt);
  unsigned int bytes_with_keep = 0;
  //v = word.tdata;
  for (unsigned int i=0; i<loop_cnt; i++) {
    //#pragma HLS PIPELINE
    //#pragma HLS UNROLL factor=loop_cnt
    //printf("DEBUG: Checking: word.tkeep=%u >> %u = %u\n", word.tkeep.to_int(), i, (word.tkeep.to_int() >> i));
    if ((word.tkeep >> i) == 0) {
      printf("WARNING: value with tkeep=0 at i=%u\n", i);
      continue; 
    }
    v.data = (ap_uint<INPUT_PTR_WIDTH>)(word.tdata >> i*8);
    v.keep = word.tkeep;
    v.last = word.tlast;
    //printf("DEBUG in storeWordToAxiStream: word = %u = 0x%16.16llX \n", v.data, v.data);
    img_in_axi_stream.write(v);
    bytes_with_keep += bytes_per_loop;
  }
  /*
  if (*processed_word_rx < IMG_PACKETS-1) {
    (*processed_word_rx)++;
  }
  else {
    printf("DEBUG in storeWordToAxiStream: WARNING - you've reached the max depth of img. Will put *processed_word_rx = 0.\n");
    *processed_word_rx = 0;
    *image_loaded = true;
  }*/
  if (*processed_bytes_rx < IMGSIZE-BYTES_PER_10GBITETHRNET_AXI_PACKET) {
    (*processed_bytes_rx) += bytes_with_keep;
  }
  else {
    printf("DEBUG in storeWordToAxiStream: WARNING - you've reached the max depth of img. Will put *processed_bytes_rx = 0.\n");
    *processed_bytes_rx = 0;
    *image_loaded = true;
  }
}


    static NetworkWord    netWord;

/*****************************************************************************
 * @brief   Store a net word to DDR memory (axi master)
 * @return Nothing.
 *****************************************************************************/
void storeWordToMem(
  //NetworkWord               word,
  //---- P0 Write Path (S2MM) -----------
  stream<DmCmd>             &soMemWrCmdP0,
  stream<DmSts>             &siMemWrStsP0,
  stream<Axis<MEMDW_512> >  &soMemWriteP0,
  //---- P1 Memory mapped ---------------
  membus_t                  *lcl_mem0,
  //---- Syncronization variables -------
  unsigned int              *processed_word_rx,
  unsigned int              *processed_bytes_rx,
  bool                      *image_loaded,
  //stream<bool>              &sImageLoaded,
  bool                      *skip_read,
  bool                      *write_chunk_to_ddr_pending,
  bool                      *ready_to_accept_new_data,
  bool                      *signal_init
)
{
    #pragma HLS INLINE

    Data_t_in v;
    v.data = 0;
    v.keep = 0;
    v.last = 0;
    const unsigned int loop_cnt = (BITS_PER_10GBITETHRNET_AXI_PACKET/INPUT_PTR_WIDTH);
    const unsigned int bytes_per_loop = (BYTES_PER_10GBITETHRNET_AXI_PACKET/loop_cnt);
    static unsigned int bytes_with_keep;
    static stream<Data_t_in> img_in_axi_stream ("img_in_axi_stream");
    #pragma HLS stream variable=img_in_axi_stream depth=65
    // reuse the unused register 'processed_word_rx' for 'ddr_addr_in'
    static unsigned int * ddr_addr_in = processed_word_rx;
    static membus_t tmp;
  
    // FIXME: Initialize to zero
    static ap_uint<32> patternWriteNum;
    static ap_uint<32> timeoutCnt;
    
    Axis<MEMDW_512>   memP0;
    DmSts             memRdStsP0;
    DmSts             memWrStsP0;
  
  
  
    printf("DEBUG: storeWordToMem, skip_read = %s, write_chunk_to_ddr_pending=%s, ready_to_accept_new_data=%s\n", 
           *skip_read?"true":"false", *write_chunk_to_ddr_pending?"true":"false", 
           *ready_to_accept_new_data?"true":"false");
    switch(fsmStateDDR) {
    
        case FSM_IDLE:
            printf("DEBUG in storeWordToMem: fsmStateDDR - FSM_IDLE\n");            
            //initalize 
            memP0.tdata = 0;
            memP0.tlast = 0;
            memP0.tkeep = 0;
            patternWriteNum = 0;
            timeoutCnt = 0;
            tmp = 0;
            bytes_with_keep = 0;
            *ready_to_accept_new_data = true;
            if ((*signal_init == true) && (enqueueFSM == PROCESSING_PACKET)) {
                fsmStateDDR = FSM_CHK_SKIP;
                *signal_init = false;
                if ((*processed_bytes_rx) == 0) {
                    (*ddr_addr_in) = 0;
                }
            }
            //if (sImageLoaded.empty()) {
            //    sImageLoaded.write(false);
            //    sImageLoaded.write(false);
            //    sImageLoaded.write(false);
            //    sImageLoaded.write(false);
            //}            
        break;
        
        case FSM_CHK_SKIP:
            printf("DEBUG in storeWordToMem: fsmStateDDR - FSM_CHK_SKIP\n");
            if (*skip_read == false) {
                printf("DEBUG in storeWordToMem: Data write = {D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
                        netWord.tdata.to_long(), netWord.tkeep.to_int(), netWord.tlast.to_int());  
                for (unsigned int i=0; i<loop_cnt; i++) {
                    //#pragma HLS PIPELINE
                    //#pragma HLS UNROLL factor=loop_cnt
                    //printf("DEBUG: Checking: netWord.tkeep=%u >> %u = %u\n", netWord.tkeep.to_int(), i, (netWord.tkeep.to_int() >> i));
                    if ((netWord.tkeep >> i) == 0) {
                        printf("WARNING: value with tkeep=0 at i=%u\n", i);
                        continue; 
                    }
                    v.data = (ap_uint<INPUT_PTR_WIDTH>)(netWord.tdata >> i*8);
                    v.keep = netWord.tkeep;
                    v.last = netWord.tlast;
                    img_in_axi_stream.write(v);
                    bytes_with_keep += bytes_per_loop;
                }
                *ready_to_accept_new_data = false;
                fsmStateDDR = FSM_CHK_PROC_BYTES;
            } 
            else {
                fsmStateDDR = FSM_CHK_WRT_CHNK_TO_DDR_PND;
            }
        break;
        
        case FSM_CHK_PROC_BYTES:
            printf("DEBUG in storeWordToMem: fsmStateDDR - FSM_CHK_PROC_BYTES, processed_bytes_rx=%u\n", *processed_bytes_rx);
            if (*processed_bytes_rx < IMGSIZE-BYTES_PER_10GBITETHRNET_AXI_PACKET) {
                (*processed_bytes_rx) += bytes_with_keep;
            }
            else {
                printf("DEBUG in storeWordToMem: WARNING - you've reached the max depth of img. Will put *processed_bytes_rx = 0.\n");
                *processed_bytes_rx = 0;
            }
            bytes_with_keep = 0;
            fsmStateDDR = FSM_CHK_WRT_CHNK_TO_DDR_PND;
            
        break;
        
        case FSM_CHK_WRT_CHNK_TO_DDR_PND :
            printf("DEBUG in storeWordToMem: fsmStateDDR - FSM_CHK_WRT_CHNK_TO_DDR_PND\n");

            // Both when we have a new word for DDR or the net stream ended (*processed_bytes_rx = 0)
            if ((*processed_bytes_rx) % BPERMDW_512 == 0) {
                printf("DEBUG in storeWordToMem: Accumulated %u net words (%u B) to complete a single DDR word\n", 
                        KWPERMDW_512, BPERMDW_512);
                *write_chunk_to_ddr_pending = true;
                fsmStateDDR = FSM_WR_PAT_CMD;
            }
            else {
                *ready_to_accept_new_data = true;
                fsmStateDDR = FSM_CHK_SKIP;
            }
        break;
           
        case FSM_WR_PAT_CMD:
            printf("DEBUG in storeWordToMem: fsmStateDDR - FSM_WR_PAT_CMD\n");
            if (*write_chunk_to_ddr_pending && !soMemWrCmdP0.full()) {
                //-- Post a memory write command to SHELL/Mem/Mp0
                soMemWrCmdP0.write(DmCmd(((*ddr_addr_in)++) * BPERMDW_512, CHECK_CHUNK_SIZE));
                patternWriteNum = 0;
                // -- Assemble a 512-bit memory word with input values from stream
                for (unsigned int i=0; i<BPERMDW_512; i++) {
                    v = img_in_axi_stream.read();
                    tmp((i+1)*INPUT_PTR_WIDTH-1, i*INPUT_PTR_WIDTH ) = v.data;
                }                    
                fsmStateDDR = FSM_WR_PAT_DATA;
            }
            else {
                *ready_to_accept_new_data = true;
                fsmStateDDR = FSM_CHK_SKIP;
            }
        break;
    
        case FSM_WR_PAT_DATA:
        printf("DEBUG in storeWordToMem: fsmStateDDR - FSM_WR_PAT_DATA\n");                
            if (!soMemWriteP0.full()) {
                //-- Write a memory word to DRAM
                memP0.tdata = tmp; // (ap_uint<512>) (currentMemPattern,currentMemPattern,currentMemPattern,currentMemPattern,currentMemPattern,currentMemPattern,currentMemPattern,currentMemPattern);
                ap_uint<8> keepVal = 0xFF;
                memP0.tkeep = (ap_uint<64>) (keepVal, keepVal, keepVal, keepVal, keepVal, keepVal, keepVal, keepVal);
                if(patternWriteNum == TRANSFERS_PER_CHUNK -1) {
                    printf("DEBUG: (patternWriteNum == TRANSFERS_PER_CHUNK -1) \n");
                    memP0.tlast = 1;
                    fsmStateDDR = FSM_WR_PAT_STS;
                }
                else {
                    memP0.tlast = 0;
                }
                //printf("DEBUG in storeWordToMem: FSM_WR_PAT_DATA write = {D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
                //    memP0.tdata.to_long(), memP0.tkeep.to_int(), memP0.tlast.to_int()); 
                soMemWriteP0.write(memP0);
                patternWriteNum++;
            }
        break;    
             
        case FSM_WR_PAT_STS:
            printf("DEBUG in storeWordToMem: fsmStateDDR - FSM_WR_PAT_STS\n");                
            if (!siMemWrStsP0.empty()) {
                printf(" 1 \n");
                //-- Get the memory write status for Mem/Mp0
                siMemWrStsP0.read(memWrStsP0);
                // TODO: handle errors on memWrStsP0
                fsmStateDDR = FSM_IDLE;
                *write_chunk_to_ddr_pending = false; // exit from loop
                if ((*processed_bytes_rx) == 0) {
                    *image_loaded = true;
                    //if (sImageLoaded.empty()) {
                    //    sImageLoaded.write(true);
                    //}
                }
            }
            else {
                printf(" 2 \n");
                timeoutCnt++;
                if (timeoutCnt >= CYCLES_UNTIL_TIMEOUT) {
                    printf(" 3 \n");
                    fsmStateDDR = FSM_IDLE;
                    *write_chunk_to_ddr_pending = false; // exit from loop but with an error
                    if ((*processed_bytes_rx) == 0) {
                        *image_loaded = true;
                    }
                }
            }
        break;            
    }
    
    //for (unsigned int i=0; i<BPERMDW_512; i++) {
    //  v = img_in_axi_stream.read();
    //  tmp((i+1)*INPUT_PTR_WIDTH-1, i*INPUT_PTR_WIDTH ) = v.data;
    //}
    // Write to DDR
    //lcl_mem0[(*ddr_addr_in)++] = tmp;
    //memcpy((membus_t  *) (lcl_mem0 + (*ddr_addr_in)++), &tmp, sizeof(membus_t));
  //}
  
  //if ((*processed_bytes_rx) == 0) {
  //  (*ddr_addr_in) = 0;
  //}
  
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
 * @param[out] image_loaded
 *
 * @return Nothing.
 ******************************************************************************/
void pRXPath(
    stream<NetworkWord>                 &siSHL_This_Data,
    stream<NetworkMetaStream>           &siNrc_meta,
    stream<NetworkMetaStream>           &sRxtoTx_Meta,
    #ifdef ENABLE_DDR
    //---- P0 Write Path (S2MM) -----------
    stream<DmCmd>                       &soMemWrCmdP0,
    stream<DmSts>                       &siMemWrStsP0,
    stream<Axis<MEMDW_512> >            &soMemWriteP0,
    //---- P1 Memory mapped ---------------
    membus_t                            *lcl_mem0,
    #else // !ENABLE_DDR
    #ifdef USE_HLSLIB_STREAM
    Stream<Data_t_in, MIN_RX_LOOPS>     &img_in_axi_stream,
    #else // !USE_HLSLIB_STREAM
    stream<Data_t_in>                   &img_in_axi_stream,
    #endif // USE_HLSLIB_STREAM
    #endif // ENABLE_DDR
    NetworkMetaStream                   meta_tmp,
    unsigned int                        *processed_word_rx,
    unsigned int                        *processed_bytes_rx,
    bool                                *image_loaded,
    //stream<bool>                        &sImageLoaded
    bool                                *skip_read,
    bool                                *write_chunk_to_ddr_pending,
    bool                                *ready_to_accept_new_data,
    bool                                *signal_init
    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    //#pragma HLS DATAFLOW interval=1
     #pragma  HLS INLINE
    //-- LOCAL VARIABLES ------------------------------------------------------
    //static NetworkWord    netWord;
    #ifdef ENABLE_DDR
    //static bool skip_read;
    //static bool write_chunk_to_ddr_pending;
    //static bool ready_to_accept_new_data;
    //static bool signal_init;
    //static bool tmp = false;
    //#pragma HLS reset variable=skip_read
    //#pragma HLS reset variable=write_chunk_to_ddr_pending
    //#pragma HLS reset variable=ready_to_accept_new_data
    //#pragma HLS reset variable=signal_init
    //#pragma HLS reset variable=tmp
    #endif
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
            #ifdef ENABLE_DDR // FIXMEL check since it was ifndef!!!
            if ((*processed_bytes_rx) == 0) {
                *write_chunk_to_ddr_pending = false;
                *ready_to_accept_new_data = false;
                *signal_init = true;
                netWord.tlast = 0;
                netWord.tkeep = 0x0;
                netWord.tdata = 0x0;
            }
            #endif        
            enqueueFSM = PROCESSING_PACKET;
        }
        *image_loaded = false;
        //if (sImageLoaded.empty()) {
        //    sImageLoaded.write(false);
        //}
        #ifdef ENABLE_DDR
        *skip_read = false;
        #endif
      break;

    case PROCESSING_PACKET:
        printf("DEBUG in pRXPath: enqueueFSM - PROCESSING_PACKET, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
        *processed_word_rx, *processed_bytes_rx);
        if ( !siSHL_This_Data.empty() 
            #ifndef ENABLE_DDR 
            && !img_in_axi_stream.full()
            #else
            || 1
            //|| (*write_chunk_to_ddr_pending == true)
            #endif
            )
        {
            #ifdef ENABLE_DDR 
         
            if (!siSHL_This_Data.empty() && 
                (*write_chunk_to_ddr_pending == false) &&
                (*ready_to_accept_new_data == true)) {
                //-- Read incoming data chunk
                netWord = siSHL_This_Data.read();
                *skip_read = false;
            }
            else {
                if ((*processed_bytes_rx) % BPERMDW_512 == 0) {
                    *skip_read = true;
                }
            }
            /*storeWordToMem( netWord,
                        soMemWrCmdP0,
                        siMemWrStsP0,
                        soMemWriteP0,
                        lcl_mem0, 
                        processed_word_rx, 
                        processed_bytes_rx, 
                        image_loaded,
                        //sImageLoaded,
                        &skip_read,
                        &write_chunk_to_ddr_pending,
                        &ready_to_accept_new_data,
                        &signal_init
                        );
            */
            
            if ((*write_chunk_to_ddr_pending == false) && (*ready_to_accept_new_data == true)) {
                if ((*processed_bytes_rx) != 0) {
                    *signal_init = true;
                }
                if (netWord.tlast == 1) {
                    enqueueFSM = WAIT_FOR_META;
                }
            }
            
            #else // ! ENABLE_DDR
            
            //-- Read incoming data chunk
            netWord = siSHL_This_Data.read();
            storeWordToAxiStream(netWord, img_in_axi_stream, processed_word_rx, processed_bytes_rx, 
                            image_loaded);
            if(netWord.tlast == 1)
            {
                enqueueFSM = WAIT_FOR_META;
            }            
            
            #endif // ENABLE_DDR
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
 * @param[in]  image_loaded
 *
 * @return Nothing.
 ******************************************************************************/
void pProcPath(
        stream<NetworkWord>                     &sRxpToTxp_Data,
        #ifdef ENABLE_DDR
        //---- P1 Memory mapped ---------------
        membus_t                                *lcl_mem0,        
        membus_t                                *lcl_mem1,
        #else // !ENABLE_DDR
        #ifdef USE_HLSLIB_STREAM
        Stream<Data_t_in, MIN_RX_LOOPS>         &img_in_axi_stream,
        Stream<Data_t_out, MIN_TX_LOOPS>        &img_out_axi_stream,
        #else // !USE_HLSLIB_STREAM
        stream<Data_t_in>                       &img_in_axi_stream,
        stream<Data_t_out>                      &img_out_axi_stream,
        #endif // USE_HLSLIB_STREAM
        #endif // ENABLE_DDR	       
        
        
        unsigned int                            *processed_word_rx,
        unsigned int                            *processed_bytes_rx, 
        bool                                    *image_loaded
        //stream<bool>                            &sImageLoaded
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
    static bool accel_called;
    static unsigned int processed_word_proc;
    Data_t_out temp;
    #ifdef ENABLE_DDR 
    static stream<Data_t_out> img_out_axi_stream ("img_out_axi_stream");
    #pragma HLS stream variable=img_out_axi_stream depth=9
    static unsigned int ddr_addr_out;
    #endif
    
  switch(HarrisFSM)
  {
    case WAIT_FOR_META: 
      printf("DEBUG in pProcPath: WAIT_FOR_META\n");
      if ( (*image_loaded) == true )
      //  if (!sImageLoaded.empty())
        {
            //if (sImageLoaded.read() == true) {
                HarrisFSM = PROCESSING_PACKET;
                //	*processed_word_rx = 0;
                //	*processed_bytes_rx = 0;
                accel_called = false;
                processed_word_proc = 0;
                #ifdef ENABLE_DDR
                ddr_addr_out = 0;
                #endif
            //}
        }
    break;

    case PROCESSING_PACKET:
      printf("DEBUG in pProcPath: PROCESSING_PACKET\n");
      #ifndef ENABLE_DDR 
      if ( !img_in_axi_stream.empty() && !img_out_axi_stream.full() )
      {
      #endif
	if (accel_called == false) {
	    #ifdef ENABLE_DDR 
	    cornerHarrisAccelMem(lcl_mem0, lcl_mem1, WIDTH, HEIGHT, Thresh, k);
	    #else // ! ENABLE_DDR
	    #ifdef FAKE_Harris
	    fakeCornerHarrisAccelStream(img_in_axi_stream, img_out_axi_stream, MIN_RX_LOOPS, MIN_TX_LOOPS);
	    #else // !FAKE_Harris
	    cornerHarrisAccelStream(img_in_axi_stream, img_out_axi_stream, WIDTH, HEIGHT, Thresh, k);
	    #endif // FAKE_Harris
	    #endif // ENABLE_DDR
	    accel_called = true;
	    HarrisFSM = HARRIS_RETURN_RESULTS;
	}
      #ifndef ENABLE_DDR
      }
      #endif
      break;
      
    #ifdef ENABLE_DDR 
    case HARRIS_RETURN_RESULTS:
      printf("DEBUG in pProcPath: HARRIS_RETURN_RESULTS\n");      
      if (accel_called == true) {
	
	  printf("DEBUG in pProcPath: Accumulated %u net words (%u B) to complete a single DDR word\n", 
	       KWPERMDW_512, BPERMDW_512);
	  
	  membus_t tmp = lcl_mem1[ddr_addr_out++];
	  
	  temp.keep = 0;
	  temp.last = 0;
	  for (unsigned int i=0; i<(MEMDW_512/OUTPUT_PTR_WIDTH); i++) {
	    #if OUTPUT_PTR_WIDTH == 64
	    ap_uint<OUTPUT_PTR_WIDTH> raw64;
	    //raw64 = tmp(i*OUTPUT_PTR_WIDTH, (i+1)*OUTPUT_PTR_WIDTH-1);
	    raw64(56,63) = tmp(i*OUTPUT_PTR_WIDTH+56, i*OUTPUT_PTR_WIDTH+63);
	    raw64(48,55) = tmp(i*OUTPUT_PTR_WIDTH+48, i*OUTPUT_PTR_WIDTH+55);
	    raw64(40,47) = tmp(i*OUTPUT_PTR_WIDTH+40, i*OUTPUT_PTR_WIDTH+47);
	    raw64(32,39) = tmp(i*OUTPUT_PTR_WIDTH+32, i*OUTPUT_PTR_WIDTH+39);
	    raw64(24,31) = tmp(i*OUTPUT_PTR_WIDTH+24, i*OUTPUT_PTR_WIDTH+31);
	    raw64(16,23) = tmp(i*OUTPUT_PTR_WIDTH+16, i*OUTPUT_PTR_WIDTH+23);
	    raw64(8 ,15) = tmp(i*OUTPUT_PTR_WIDTH+8 , i*OUTPUT_PTR_WIDTH+15);
	    raw64(0 ,7 ) = tmp(i*OUTPUT_PTR_WIDTH   , i*OUTPUT_PTR_WIDTH+7);
	    temp.data = raw64;
	    #endif 
	    img_out_axi_stream.write(temp);
	  }
	  
	  HarrisFSM = HARRIS_RETURN_RESULTS_FWD;
	//}
      }
      break;
    case HARRIS_RETURN_RESULTS_FWD:
      printf("DEBUG in pProcPath: HARRIS_RETURN_RESULTS_FWD\n");
      if ( !img_out_axi_stream.empty() && !sRxpToTxp_Data.full() )
      {
	
	temp = img_out_axi_stream.read();
	if ( img_out_axi_stream.empty() ) {
	  HarrisFSM = HARRIS_RETURN_RESULTS;
	}
	
	if (processed_word_proc++ == MIN_TX_LOOPS-1)
	{
	  temp.last = 1;
	  HarrisFSM = WAIT_FOR_META;
	  accel_called = false;
	}
	else
	{
	  temp.last = 0;
	}
	//TODO: find why Vitis kernel does not set keep and last by itself
	temp.keep = 255;
	newWord = NetworkWord(temp.data, temp.keep, temp.last); 
	sRxpToTxp_Data.write(newWord);
      }
      break;

    #else // ! ENABLE_DDR
    case HARRIS_RETURN_RESULTS:
        printf("DEBUG in pProcPath: HARRIS_RETURN_RESULTS\n");
        if ( !img_out_axi_stream.empty() && !sRxpToTxp_Data.full() )
        {
	
            temp = img_out_axi_stream.read();
            if ( img_out_axi_stream.empty() )
            //if (processed_word_proc++ == MIN_TX_LOOPS-1)
            {
                temp.last = 1;
                HarrisFSM = WAIT_FOR_META;
                accel_called = false;
            }
            else
            {
                temp.last = 0;
            }
            //TODO: find why Vitis kernel does not set keep and last by itself
            temp.keep = 255;
            newWord = NetworkWord(temp.data, temp.keep, temp.last); 
            sRxpToTxp_Data.write(newWord);
        }
        break;
    #endif // ENABLE_DDR
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

        // Forcing the SHELL to wait for tlast
        meta_out_stream.tdata.len = 0;

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
 * @brief   Main process of the Harris Application 
 * directives.
 * @deprecated  This functions is using deprecated AXI stream interface 
 * @return Nothing.
 *****************************************************************************/
void harris(

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

// Bundling: SHELL / Role / Mem / Mp0 / Read Interface
// #pragma HLS INTERFACE axis register both port=soMemRdCmdP0
// #pragma HLS INTERFACE axis register both port=siMemRdStsP0
// #pragma HLS INTERFACE axis register both port=siMemReadP0

// #pragma HLS DATA_PACK variable=soMemRdCmdP0 instance=soMemRdCmdP0
// #pragma HLS DATA_PACK variable=siMemRdStsP0 instance=siMemRdStsP0

// Bundling: SHELL / Role / Mem / Mp0 / Write Interface
#pragma HLS INTERFACE axis register both port=soMemWrCmdP0
#pragma HLS INTERFACE axis register both port=siMemWrStsP0
#pragma HLS INTERFACE axis register both port=soMemWriteP0

#pragma HLS DATA_PACK variable=soMemWrCmdP0 instance=soMemWrCmdP0
#pragma HLS DATA_PACK variable=siMemWrStsP0 instance=siMemWrStsP0    
    
const unsigned int ddr_mem_depth = TOTMEMDW_512;

// Mapping LCL_MEM0 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem0 bundle=moMEM_Mp1\
  max_read_burst_length=256  max_write_burst_length=256 offset=direct \
  num_read_outstanding=16 num_write_outstanding=16 latency=52

// Mapping LCL_MEM1 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem1 bundle=moMEM_Mp1 \
  max_read_burst_length=256  max_write_burst_length=256 offset=direct \
  num_read_outstanding=16 num_write_outstanding=16 latency=52

#endif

  //-- LOCAL VARIABLES ------------------------------------------------------
  NetworkMetaStream  meta_tmp = NetworkMetaStream();
  static stream<NetworkWord>       sRxpToTxp_Data("sRxpToTxP_Data"); // FIXME: works even with no static
  static stream<NetworkMetaStream> sRxtoTx_Meta("sRxtoTx_Meta");
  static unsigned int processed_word_rx;
  static unsigned int processed_bytes_rx;
  static unsigned int processed_word_tx;
  static bool image_loaded;
  //static stream<bool> sImageLoaded("sImageLoaded");
  static bool skip_read;
  static bool write_chunk_to_ddr_pending;
  static bool ready_to_accept_new_data;
  static bool signal_init;
  const int img_in_axi_stream_depth = MIN_RX_LOOPS;
  const int img_out_axi_stream_depth = MIN_TX_LOOPS;
  const int tot_transfers = TOT_TRANSFERS;
#ifndef ENABLE_DDR
#ifdef USE_HLSLIB_DATAFLOW
  static hlslib::Stream<Data_t_in,  MIN_RX_LOOPS> img_in_axi_stream ("img_in_axi_stream");
  static hlslib::Stream<Data_t_out, MIN_TX_LOOPS> img_out_axi_stream ("img_out_axi_stream");
#else
  static stream<Data_t_in>  img_in_axi_stream ("img_in_axi_stream" );
  static stream<Data_t_out> img_out_axi_stream("img_out_axi_stream");
#endif
#endif
  *po_rx_ports = 0x1; //currently work only with default ports...

  
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS stream variable=sRxtoTx_Meta depth=tot_transfers
#pragma HLS reset variable=enqueueFSM
#pragma HLS reset variable=dequeueFSM
#pragma HLS reset variable=HarrisFSM
#pragma HLS reset variable=processed_word_rx
#pragma HLS reset variable=processed_word_tx
#pragma HLS reset variable=processed_bytes_rx
#pragma HLS reset variable=image_loaded
//#pragma HLS stream variable=sImageLoaded depth=1
#pragma HLS reset variable=skip_read
#pragma HLS reset variable=write_chunk_to_ddr_pending
#pragma HLS reset variable=ready_to_accept_new_data
#pragma HLS reset variable=signal_init

#ifndef ENABLE_DDR
#pragma HLS stream variable=img_in_axi_stream depth=img_in_axi_stream_depth
#pragma HLS stream variable=img_out_axi_stream depth=img_out_axi_stream_depth
#else
#pragma HLS reset variable=fsmStateDDR
#endif

#pragma HLS DATAFLOW


#ifdef USE_HLSLIB_DATAFLOW
  /*! @copybrief harris()
   *  Harris is enabled with hlslib support
   */
  /*! @copydoc harris()
   * Use this snippet to early check for C++ errors related to dataflow and bounded streams (empty 
   * and full) during simulation. It can also be both synthesized and used in co-simulation.
   * Practically we use hlslib when we want to run simulation as close as possible to the HW, by 
   * executing all functions of dataflow in thread-safe parallel executions, i.e the function 
   * HLSLIB_DATAFLOW_FINALIZE() acts as a barrier for the threads spawned to serve every function 
   * called in HLSLIB_DATAFLOW_FUNCTION(func, args...).
   */
   /*! @copydetails harris()
   * hlslib is a collection of C++ headers, CMake files, and examples, aimed at improving the 
   * quality of life of HLS developers. More info at: https://github.com/definelicht/hlslib
   */
  // Dataflow functions running in parallel
  HLSLIB_DATAFLOW_INIT();
  
  HLSLIB_DATAFLOW_FUNCTION(pRXPath, 
			   siSHL_This_Data,
			   siNrc_meta,
			   sRxtoTx_Meta,
#ifdef ENABLE_DDR
                //---- P0 Write Path (S2MM) -----------
                soMemWrCmdP0,
                siMemWrStsP0,
                soMemWriteP0,
                // ---- P1 Memory mapped --------------
                lcl_mem0,
#else
			   img_in_axi_stream,
#endif
			   meta_tmp,
			   &processed_word_rx,
			   &processed_bytes_rx,
			   &image_loaded
			  );
  
  HLSLIB_DATAFLOW_FUNCTION(pProcPath,
			   sRxpToTxp_Data,
#ifdef ENABLE_DDR
			   lcl_mem0,
			   lcl_mem1,
#else
			   img_in_axi_stream,
			   img_out_axi_stream,
#endif
		           &processed_word_rx,
			   &processed_bytes_rx,
		           &image_loaded
			  ); 

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
#ifdef ENABLE_DDR
    //---- P0 Write Path (S2MM) -----------
    soMemWrCmdP0,
    siMemWrStsP0,
    soMemWriteP0,
    // ---- P1 Memory mapped --------------
    lcl_mem0,
#else
    img_in_axi_stream,
#endif
    meta_tmp,
    &processed_word_rx,
    &processed_bytes_rx,
    &image_loaded,
    //sImageLoaded
    &skip_read,
    &write_chunk_to_ddr_pending,
    &ready_to_accept_new_data,
    &signal_init    
    );

#ifdef ENABLE_DDR
    storeWordToMem( 
                    soMemWrCmdP0,
                    siMemWrStsP0,
                    soMemWriteP0,
                    lcl_mem0, 
                    &processed_word_rx, 
                    &processed_bytes_rx, 
                    &image_loaded,
                    //sImageLoaded,
                    &skip_read,
                    &write_chunk_to_ddr_pending,
                    &ready_to_accept_new_data,
                    &signal_init
                    );
#endif 
 
  pProcPath(sRxpToTxp_Data,
#ifdef ENABLE_DDR
        lcl_mem0,
        lcl_mem1,
#else
        img_in_axi_stream,
        img_out_axi_stream,
#endif
        &processed_word_rx,
        &processed_bytes_rx,
        &image_loaded
        //sImageLoaded
        );  

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
