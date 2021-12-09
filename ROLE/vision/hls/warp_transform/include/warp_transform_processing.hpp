/*****************************************************************************
 * @file       warp_transform_processing.hpp
 * @brief      The processing function for the warp_transform filte
 * @author     DCO
 * @date       September 2021
 *----------------------------------------------------------------------------
 *
 * @details      A WarpTransform Filter FSM for the processing of data
 *
 * @deprecated   
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup WarpTransformHLS
 * @addtogroup WarpTransformHLS
 * \{
 *****************************************************************************/  
#ifndef _ROLE_WARPTRANSFORM_PROCESSING_HPP_
#define _ROLE_WARPTRANSFORM_PROCESSING_HPP_


#define FSM_PROCESSING_WAIT_FOR_META 0
#define FSM_PROCESSING_PCKT_PROC 1
#define FSM_PROCESSING_STOP 2
#define FSM_PROCESSING_START 3
#define FSM_PROCESSING_BURST_READING 4
#define FSM_PROCESSING_DATAFLOW_WRITE 5
#define FSM_PROCESSING_DATAFLOW_READ 6
#define FSM_PROCESSING_OUTPUT 7
#define FSM_PROCESSING_OUTPUT_2 8
#define FSM_PROCESSING_OUTPUT_3 9
#define FSM_PROCESSING_OUTPUT_4 10
#define FSM_PROCESSING_OUTPUT_5 11
#define FSM_PROCESSING_CONTINUOUS_RUN 12
#define FSM_PROCESSING_WAIT_FOR_DDR_CONTROLLER_EMPTYNESS 13
#define ProcessingFsmType uint8_t


/*****************************************************************************
 * @brief Processing Path - Main processing FSM for Vitis kernels.
 *
 * @param[out] sRxpToTxp_Data
 * @param[in]  img_in_axi_stream
 * @param[in]  img_out_axi_stream
 * @param[out] processed_word_rx
 * @param[in]  sImageLoaded
 *
 * @return Nothing.
 ******************************************************************************/
#ifndef ENABLE_DDR
template <typename TimgIn=ap_uint<INPUT_PTR_WIDTH>, typename TimgOut=ap_uint<OUTPUT_PTR_WIDTH>>
#endif // ENABLE_DDR
void pProcPath(
        stream<NetworkWord>                   &sRxpToTxp_Data,
        #ifdef ENABLE_DDR
        //---- P1 Memory mapped ---------------
        membus_t                              *lcl_mem0,        
        membus_t                              *lcl_mem1,
        #else // !ENABLE_DDR
        stream<TimgIn>                         &img_in_axi_stream,
        stream<TimgOut>                        &img_out_axi_stream,
        #endif // ENABLE_DDR	       
        stream<bool>                           &sImageLoaded,
        hls::stream<img_meta_t>                &sInImgRows,
        hls::stream<img_meta_t>                &sInImgCols,
        hls::stream<img_meta_t>                &sInImgChan,
        float                                  tx_matrix[TRANSFORM_MATRIX_DIM]
        )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS INLINE off
    #pragma HLS pipeline II=1
    
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord newWord;
    uint16_t Thresh = 442;
    float K = 0.04;
    uint16_t k = K * (1 << 16); // Convert to Q0.16 format
    static bool accel_called;
    static unsigned int processed_word_proc;
    static unsigned int timeoutCntAbs;
    static unsigned int cnt_i;
    static membus_t tmp;
    Data_t_out temp;
    #ifdef ENABLE_DDR 
    //static stream<ap_uint<OUTPUT_PTR_WIDTH>> img_out_axi_stream ("img_out_axi_stream");
    //#pragma HLS stream variable=img_out_axi_stream depth=9
    static unsigned int ddr_addr_out;
    #pragma HLS reset variable=ddr_addr_out
    #endif
    static PacketFsmType WarpTransformFSM = WAIT_FOR_META;
    #pragma HLS reset variable=WarpTransformFSM

    #pragma HLS reset variable=accel_called  
    #pragma HLS reset variable=processed_word_proc  
    #pragma HLS reset variable=timeoutCntAbs  
    #pragma HLS reset variable=cnt_i  
    #pragma HLS reset variable=tmp  
    #pragma HLS reset variable=temp 

    static img_meta_t img_rows=0; 
    static img_meta_t img_cols=0; 
    static img_meta_t img_chan=0; 
    #pragma HLS reset variable=img_rows    
    #pragma HLS reset variable=img_cols    
    #pragma HLS reset variable=img_chan  

    
  switch(WarpTransformFSM)
  {
    case WAIT_FOR_META: 
      printf("DEBUG in pProcPath: WAIT_FOR_META\n");
      if (!sImageLoaded.empty())
        {
            if (sImageLoaded.read() == true) {
                WarpTransformFSM = PROCESSING_PACKET;
                accel_called = false;
                processed_word_proc = 0;
                #ifdef ENABLE_DDR
                ddr_addr_out = 0;
                timeoutCntAbs = 0;
                cnt_i = 0;
                #endif
                img_rows  = sInImgRows.read();
                img_cols  = sInImgCols.read();
                img_chan  = sInImgChan.read();
            }
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
            warp_transformAccelMem(lcl_mem0, lcl_mem1, img_rows, img_cols, tx_matrix);
	    #else // ! ENABLE_DDR
	    #ifdef FAKE_WarpTransform
            fakeWarpTransformAccelStream(img_in_axi_stream, img_out_axi_stream, MIN_RX_LOOPS, MIN_TX_LOOPS, tx_matrix);
	    #else // !FAKE_WarpTransform
            warpTransformAccelStream(img_in_axi_stream, img_out_axi_stream, img_rows, img_cols, tx_matrix);
	    #endif // FAKE_WarpTransform
	    #endif // ENABLE_DDR
            accel_called = true;
            WarpTransformFSM = WARPTRANSFORM_RETURN_RESULTS;
        }
        #ifndef ENABLE_DDR
        }
        #endif
    break;
      
    #ifdef ENABLE_DDR 
    case WARPTRANSFORM_RETURN_RESULTS:
      printf("DEBUG in pProcPath: WARPTRANSFORM_RETURN_RESULTS, ddr_addr_out=%u\n", ddr_addr_out);      
      if (accel_called == true) {
	
        printf("DEBUG in pProcPath: Accumulated %u net words (%u B) to complete a single DDR word\n", 
	       KWPERMDW_512, BPERMDW_512);
            tmp = lcl_mem1[ddr_addr_out];
            ddr_addr_out++;
            WarpTransformFSM = WARPTRANSFORM_RETURN_RESULTS_ABSORB_DDR_LAT;
            timeoutCntAbs = 0;
      }
    break;
    
    case WARPTRANSFORM_RETURN_RESULTS_ABSORB_DDR_LAT:
      printf("DEBUG in pProcPath: WARPTRANSFORM_RETURN_RESULTS_ABSORB_DDR_LAT [%u out of %u]\n", timeoutCntAbs, DDR_LATENCY);        
        if (timeoutCntAbs++ == DDR_LATENCY) {
            WarpTransformFSM = WARPTRANSFORM_RETURN_RESULTS_FWD; //WARPTRANSFORM_RETURN_RESULTS_UNPACK;
            cnt_i = 0;
        }
    break;
    case WARPTRANSFORM_RETURN_RESULTS_FWD: 
      printf("DEBUG in pProcPath: WARPTRANSFORM_RETURN_RESULTS_FWD\n");
      //if ( !img_out_axi_stream.empty() && !sRxpToTxp_Data.full() ) {
      if ( (cnt_i <= (MEMDW_512/OUTPUT_PTR_WIDTH) - 1) && !sRxpToTxp_Data.full() ) {
          
        //temp.data = img_out_axi_stream.read();
        temp.data(0 ,63) = tmp(cnt_i*OUTPUT_PTR_WIDTH   , cnt_i*OUTPUT_PTR_WIDTH+63);
        if (processed_word_proc++ == MIN_TX_LOOPS-1) {
            temp.last = 1;
            WarpTransformFSM = WAIT_FOR_META;
        }
        else {
            temp.last = 0;
        }
        //TODO: find why Vitis kernel does not set keep and last by itself
        temp.keep = 255;
        newWord = NetworkWord(temp.data, temp.keep, temp.last); 
        sRxpToTxp_Data.write(newWord);
        cnt_i++;
      }
      else {
        WarpTransformFSM = WARPTRANSFORM_RETURN_RESULTS;
      }
    
    break;

    #else // ! ENABLE_DDR
    case WARPTRANSFORM_RETURN_RESULTS:
        printf("DEBUG in pProcPath: WARPTRANSFORM_RETURN_RESULTS\n");
        if ( !img_out_axi_stream.empty() && !sRxpToTxp_Data.full() )
        {
	
            temp.data = img_out_axi_stream.read();
            if ( img_out_axi_stream.empty() )
            //if (processed_word_proc++ == MIN_TX_LOOPS-1)
            {
              temp.last = 1;
              WarpTransformFSM = WAIT_FOR_META;
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
#endif //_ROLE_WARPTRANSFORM_PROCESSING_HPP_
