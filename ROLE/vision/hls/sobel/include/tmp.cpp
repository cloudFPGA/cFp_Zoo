

PacketFsmType  enqueueRxToStrFSM   = WAIT_FOR_META;
PacketFsmType  enqueueStrToDdrFSM  = WAIT_FOR_META;
PacketFsmType  SobelFSM   = WAIT_FOR_META;

#ifdef ENABLE_DDR
#if TRANSFERS_PER_CHUNK_DIVEND == 0
#define TRANSFERS_PER_CHUNK_LAST_BURST TRANSFERS_PER_CHUNK
#else
#define TRANSFERS_PER_CHUNK_LAST_BURST TRANSFERS_PER_CHUNK_DIVEND
#endif
#endif

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
        stream<ap_uint<INPUT_PTR_WIDTH>>        &img_in_axi_stream,
        stream<ap_uint<OUTPUT_PTR_WIDTH>>       &img_out_axi_stream,
        #endif // USE_HLSLIB_STREAM
        #endif // ENABLE_DDR	       
        
        
        stream<bool>                            &sImageLoaded
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
    ap_uint<OUTPUT_PTR_WIDTH> raw64;
    Data_t_out temp;
    #ifdef ENABLE_DDR 
    //static stream<ap_uint<OUTPUT_PTR_WIDTH>> img_out_axi_stream ("img_out_axi_stream");
    //#pragma HLS stream variable=img_out_axi_stream depth=9
    static unsigned int ddr_addr_out;
    #pragma HLS reset variable=ddr_addr_out
    #endif
    
    #pragma HLS reset variable=accel_called  
    #pragma HLS reset variable=processed_word_proc  
    #pragma HLS reset variable=timeoutCntAbs  
    #pragma HLS reset variable=cnt_i  
    #pragma HLS reset variable=tmp  
    #pragma HLS reset variable=raw64  
    #pragma HLS reset variable=temp  
    
  switch(SobelFSM)
  {
    case WAIT_FOR_META: 
      printf("DEBUG in pProcPath: WAIT_FOR_META\n");
      if (!sImageLoaded.empty())
        {
            if (sImageLoaded.read() == true) {
                SobelFSM = PROCESSING_PACKET;
                accel_called = false;
                processed_word_proc = 0;
                #ifdef ENABLE_DDR
                ddr_addr_out = 0;
                timeoutCntAbs = 0;
                cnt_i = 0;
                #endif
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
            medianBlurAccelMem(lcl_mem0, lcl_mem1, WIDTH, HEIGHT);
	    #else // ! ENABLE_DDR
	    #ifdef FAKE_Sobel
            fakeSobelAccelStream(img_in_axi_stream, img_out_axi_stream, MIN_RX_LOOPS, MIN_TX_LOOPS);
	    #else // !FAKE_Sobel
            medianBlurAccelStream(img_in_axi_stream, img_out_axi_stream, WIDTH, HEIGHT);
	    #endif // FAKE_Sobel
	    #endif // ENABLE_DDR
            accel_called = true;
            SobelFSM = SOBEL_RETURN_RESULTS;
        }
        #ifndef ENABLE_DDR
        }
        #endif
    break;
      
    #ifdef ENABLE_DDR 
    case SOBEL_RETURN_RESULTS:
      printf("DEBUG in pProcPath: SOBEL_RETURN_RESULTS, ddr_addr_out=%u\n", ddr_addr_out);      
      if (accel_called == true) {
	
        printf("DEBUG in pProcPath: Accumulated %u net words (%u B) to complete a single DDR word\n", 
	       KWPERMDW_512, BPERMDW_512);
            tmp = lcl_mem1[ddr_addr_out];
            ddr_addr_out++;
            SobelFSM = SOBEL_RETURN_RESULTS_ABSORB_DDR_LAT;
            timeoutCntAbs = 0;
      }
    break;
    
    case SOBEL_RETURN_RESULTS_ABSORB_DDR_LAT:
      printf("DEBUG in pProcPath: SOBEL_RETURN_RESULTS_ABSORB_DDR_LAT [%u out of %u]\n", timeoutCntAbs, DDR_LATENCY);        
        if (timeoutCntAbs++ == DDR_LATENCY) {
            SobelFSM = SOBEL_RETURN_RESULTS_FWD; //SOBEL_RETURN_RESULTS_UNPACK;
            cnt_i = 0;
        }
    break;
    /*
    case SOBEL_RETURN_RESULTS_UNPACK:
      printf("DEBUG in pProcPath: SOBEL_RETURN_RESULTS_UNPACK, cnt_i=%u\n", cnt_i);        
        //for (unsigned int cnt_i=0; cnt_i<(MEMDW_512/OUTPUT_PTR_WIDTH); cnt_i++) {
            #if OUTPUT_PTR_WIDTH == 64
            raw64(0 ,63) = tmp(cnt_i*OUTPUT_PTR_WIDTH   , cnt_i*OUTPUT_PTR_WIDTH+63);
            #endif
            if ( !img_out_axi_stream.full() ) {
                img_out_axi_stream.write(raw64);
            }
            if (cnt_i == (MEMDW_512/OUTPUT_PTR_WIDTH) - 1) {
                SobelFSM = SOBEL_RETURN_RESULTS_FWD;
            }
            cnt_i++;
        //}
    break;
    */
    case SOBEL_RETURN_RESULTS_FWD: 
      printf("DEBUG in pProcPath: SOBEL_RETURN_RESULTS_FWD\n");
      //if ( !img_out_axi_stream.empty() && !sRxpToTxp_Data.full() ) {
      if ( (cnt_i <= (MEMDW_512/OUTPUT_PTR_WIDTH) - 1) && !sRxpToTxp_Data.full() ) {
          
        //temp.data = img_out_axi_stream.read();
        temp.data(0 ,63) = tmp(cnt_i*OUTPUT_PTR_WIDTH   , cnt_i*OUTPUT_PTR_WIDTH+63);
        if (processed_word_proc++ == MIN_TX_LOOPS-1) {
            temp.last = 1;
            SobelFSM = WAIT_FOR_META;
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
        SobelFSM = SOBEL_RETURN_RESULTS;
      }
    
    break;

    #else // ! ENABLE_DDR
    case SOBEL_RETURN_RESULTS:
        printf("DEBUG in pProcPath: SOBEL_RETURN_RESULTS\n");
        if ( !img_out_axi_stream.empty() && !sRxpToTxp_Data.full() )
        {
	
            temp.data = img_out_axi_stream.read();
            if ( img_out_axi_stream.empty() )
            //if (processed_word_proc++ == MIN_TX_LOOPS-1)
            {
                temp.last = 1;
                SobelFSM = WAIT_FOR_META;
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
 * @brief Receive Path - From SHELL to THIS.
 *
 * @param[in]  siSHL_This_Data
 * @param[in]  siNrc_meta
 * @param[out] sRxtoTx_Meta
 * @param[out] meta_tmp
 *
 * @return Nothing.
 ******************************************************************************/
void pRXPathNetToStream(
    stream<NetworkWord>                 &siSHL_This_Data,
    stream<NetworkMetaStream>           &siNrc_meta,
    stream<NetworkMetaStream>           &sRxtoTx_Meta,
    stream<ap_uint<MEMDW_512>>          &img_in_axi_stream,
    stream<bool>                        &sMemBurstRx
    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS INLINE off
    #pragma HLS pipeline II=1
    //#pragma HLS interface ap_ctrl_none port=return   
    
    //-- LOCAL VARIABLES ------------------------------------------------------
    static NetworkWord  netWord;
    const unsigned int  loop_cnt = (MEMDW_512/BITS_PER_10GBITETHRNET_AXI_PACKET);
    NetworkMetaStream   meta_tmp;
    static ap_uint<MEMDW_512> v = 0;
    static unsigned int cnt_wr_stream = 0, cnt_wr_burst = 0;
//    static stream<ap_uint<MEMDW_512>> img_in_axi_stream ("img_in_axi_stream");
//    const unsigned int img_in_axi_stream_depth = TRANSFERS_PER_CHUNK; // the AXI burst size
//    #pragma HLS stream variable=img_in_axi_stream depth=img_in_axi_stream_depth
      
    #pragma HLS reset variable=cnt_wr_stream
    #pragma HLS reset variable=cnt_wr_burst

    switch(enqueueRxToStrFSM)
    {
    case WAIT_FOR_META:
        printf("DEBUG in pRXPathNetToStream: enqueueRxToStrFSM - WAIT_FOR_META\n");
        
        if ( !siNrc_meta.empty() && !sRxtoTx_Meta.full() )
        {
            meta_tmp = siNrc_meta.read();
            meta_tmp.tlast = 1; //just to be sure...
            sRxtoTx_Meta.write(meta_tmp);
            enqueueRxToStrFSM = PROCESSING_PACKET;
        }
        break;

    case PROCESSING_PACKET:
        printf("DEBUG in pRXPathNetToStream: enqueueRxToStrFSM - PROCESSING_PACKET\n");
        if ( !siSHL_This_Data.empty() && !img_in_axi_stream.full())
        {
            //-- Read incoming data chunk
            netWord = siSHL_This_Data.read();
            printf("DEBUG in pRXPathNetToStream: Data write = {D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
               netWord.tdata.to_long(), netWord.tkeep.to_int(), netWord.tlast.to_int());            
            //enqueueRxToStrFSM = LOAD_IN_STREAM;
            if ((netWord.tkeep >> cnt_wr_stream) == 0) {
                printf("WARNING: value with tkeep=0 at cnt_wr_stream=%u\n", cnt_wr_stream);
                //continue;
            }
            v(cnt_wr_stream*64, (cnt_wr_stream+1)*64-1) = netWord.tdata(0,63);
            if ((cnt_wr_stream++ == loop_cnt-1) || (netWord.tlast == 1)) {
                // std::cout << std::hex << v << std::endl; // print hexadecimal value
                std::cout << "DEBUG in pRXPathNetToStream: Pushing to img_in_axi_stream :" << std::hex << v << std::endl;
                img_in_axi_stream.write(v);
                if ((cnt_wr_burst++ == TRANSFERS_PER_CHUNK-1) || (netWord.tlast == 1)) {
                    if (!sMemBurstRx.full()) {
                        sMemBurstRx.write(true);
                    }
                    cnt_wr_burst = 0;
                }
                if (netWord.tlast == 1) {                
                    enqueueRxToStrFSM = WAIT_FOR_META;
                }
                cnt_wr_stream = 0;
            }
        }
        break;
    }
}




/*****************************************************************************
 * @brief Receive Path - From SHELL to THIS.
 *
 * @param[in]  siSHL_This_Data
 * @param[in]  siNrc_meta
 * @param[out] sRxtoTx_Meta
 * @param[out] img_in_axi_stream
 * @param[out] processed_word
 * @param[out] sImageLoaded
 *
 * @return Nothing.
 ******************************************************************************/
void pRXPathStreamToDDR(
    stream<ap_uint<MEMDW_512>>          &img_in_axi_stream,
    stream<bool>                        &sMemBurstRx,    
    //---- P0 Write Path (S2MM) -----------
    stream<DmCmd>                       &soMemWrCmdP0,
    stream<DmSts>                       &siMemWrStsP0,
    stream<Axis<MEMDW_512> >            &soMemWriteP0,
    //---- P1 Memory mapped ---------------
    stream<bool>                        &sImageLoaded
    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS INLINE off
    #pragma HLS pipeline II=1
    //#pragma HLS interface ap_ctrl_none port=return
    
    //-- LOCAL VARIABLES ------------------------------------------------------
    static ap_uint<MEMDW_512> v = 0;
    const unsigned int loop_cnt = (MEMDW_512/BITS_PER_10GBITETHRNET_AXI_PACKET);
    const unsigned int bytes_per_loop = (BYTES_PER_10GBITETHRNET_AXI_PACKET*loop_cnt);
    static unsigned int cur_transfers_per_chunk;
    static unsigned int cnt_wr_stream, cnt_wr_img_loaded;
//    static stream<ap_uint<MEMDW_512>> img_in_axi_stream ("img_in_axi_stream");
//    const unsigned int img_in_axi_stream_depth = TRANSFERS_PER_CHUNK; // the AXI burst size
//    #pragma HLS stream variable=img_in_axi_stream depth=img_in_axi_stream_depth
    static unsigned int ddr_addr_in; 
  
    // FIXME: Initialize to zero
    static ap_uint<32> patternWriteNum;
    static ap_uint<32> timeoutCnt;
    
    static Axis<MEMDW_512>   memP0;
    static DmSts             memWrStsP0;    
    static unsigned int      processed_bytes_rx;
     
    #pragma HLS reset variable=cur_transfers_per_chunk
    #pragma HLS reset variable=cnt_wr_stream
    #pragma HLS reset variable=cnt_wr_img_loaded    
    #pragma HLS reset variable=ddr_addr_in
    #pragma HLS reset variable=patternWriteNum
    #pragma HLS reset variable=timeoutCnt
    #pragma HLS reset variable=memP0
    #pragma HLS reset variable=memWrStsP0    
    
    switch(enqueueStrToDdrFSM)
    {
    case WAIT_FOR_META:
        printf("DEBUG in pRXPathStreamToDDR: enqueueStrToDdrFSM - WAIT_FOR_META, processed_bytes_rx=%u\n",
               processed_bytes_rx);
        
        if ( !img_in_axi_stream.empty() )
        {
            if ((processed_bytes_rx) == 0) {
                memP0.tdata = 0;
                memP0.tlast = 0;
                memP0.tkeep = 0;
                patternWriteNum = 0;
                timeoutCnt = 0;
                cur_transfers_per_chunk = 0;                
                ddr_addr_in = 0;
                cnt_wr_stream = 0;
                v = 0;
                memWrStsP0.tag = 0;
                memWrStsP0.interr = 0;
                memWrStsP0.decerr = 0;
                memWrStsP0.slverr = 0;
                memWrStsP0.okay = 0;
            }
            enqueueStrToDdrFSM = FSM_CHK_PROC_BYTES;
        }
        break;

    case FSM_CHK_PROC_BYTES:
        printf("DEBUG in pRXPathStreamToDDR: enqueueStrToDdrFSM - FSM_CHK_PROC_BYTES, processed_bytes_rx=%u\n", processed_bytes_rx);
        if (processed_bytes_rx < IMGSIZE-bytes_per_loop) {
            (processed_bytes_rx) += bytes_per_loop;
        }
        else {
            printf("DEBUG in pRXPathStreamToDDR: WARNING - you've reached the max depth of img. Will put processed_bytes_rx = 0.\n");
            processed_bytes_rx = 0;
        }
        enqueueStrToDdrFSM = FSM_WR_PAT_CMD;
    break;

case FSM_WR_PAT_CMD:
    printf("DEBUG in pRXPathStreamToDDR: enqueueStrToDdrFSM - FSM_WR_PAT_CMD\n");
    if ( !soMemWrCmdP0.full() ) {
        //-- Post a memory write command to SHELL/Mem/Mp0
        if (processed_bytes_rx == 0){
            cur_transfers_per_chunk = TRANSFERS_PER_CHUNK_LAST_BURST;
        }
        else {
            cur_transfers_per_chunk = TRANSFERS_PER_CHUNK;
        }
        if (patternWriteNum == 0) { // Write cmd only the fitst time of every burst
            soMemWrCmdP0.write(DmCmd(ddr_addr_in * BPERMDW_512, cur_transfers_per_chunk*BPERMDW_512)); // Byte-addresable
        }
        ddr_addr_in++;
        enqueueStrToDdrFSM = FSM_WR_PAT_LOAD;
    }
    break;

case FSM_WR_PAT_LOAD:
    printf("DEBUG in pRXPathStreamToDDR: enqueueStrToDdrFSM - FSM_WR_PAT_LOAD\n");
    // -- Assemble a 512-bit memory word with input values from stream
    if (patternWriteNum++ >= cur_transfers_per_chunk - 1) {
        if (!sMemBurstRx.empty()) {
            if (sMemBurstRx.read() == true) {
                patternWriteNum = 0;
                enqueueStrToDdrFSM = FSM_WR_PAT_DATA;
            }
        }
    }
    else {
        if((processed_bytes_rx) == 0) {
            enqueueStrToDdrFSM = WAIT_FOR_META;
        }
        else {
            enqueueStrToDdrFSM = FSM_CHK_PROC_BYTES;
        }
    }
    break;
    
case FSM_WR_PAT_DATA:
    printf("DEBUG in pRXPathStreamToDDR: enqueueStrToDdrFSM - FSM_WR_PAT_DATA\n");
    if (!soMemWriteP0.full()) {
        //-- Write a memory word to DRAM
        if (!img_in_axi_stream.empty()) {
            memP0.tdata = img_in_axi_stream.read();
            ap_uint<8> keepVal = 0xFF;
            memP0.tkeep = (ap_uint<64>) (keepVal, keepVal, keepVal, keepVal, keepVal, keepVal, keepVal, keepVal);
            if (patternWriteNum++ == cur_transfers_per_chunk - 1) {
                printf("DEBUG: (patternWriteNum == cur_transfers_per_chunk -1) \n");
                memP0.tlast = 1;
                cnt_wr_img_loaded = 0;
                timeoutCnt = 0;
                patternWriteNum = 0;
                enqueueStrToDdrFSM = FSM_WR_PAT_STS_A;
            }
            else {
                memP0.tlast = 0;
            }
            std::cout << "DEBUG in pRXPathStreamToDDR: Pushing to soMemWriteP0 :" << std::hex << memP0.tdata << std::endl;
            soMemWriteP0.write(memP0);
        }
    }
    break;

case FSM_WR_PAT_STS_A:
    printf("DEBUG in pRXPathStreamToDDR: enqueueStrToDdrFSM - FSM_WR_PAT_STS_A\n");
    if (!siMemWrStsP0.empty()) {
        printf(" 1 \n");
        //-- Get the memory write status for Mem/Mp0
        siMemWrStsP0.read(memWrStsP0);
        enqueueStrToDdrFSM = FSM_WR_PAT_STS_B;
    }
    else {
        if (timeoutCnt++ >= CYCLES_UNTIL_TIMEOUT) {
            memWrStsP0.tag = 0;
            memWrStsP0.interr = 0;
            memWrStsP0.decerr = 0;
            memWrStsP0.slverr = 0;
            memWrStsP0.okay = 0;           
            enqueueStrToDdrFSM = FSM_WR_PAT_STS_B;
        }
    }
    break;

case FSM_WR_PAT_STS_B:
    printf("DEBUG in pRXPathStreamToDDR: enqueueStrToDdrFSM - FSM_WR_PAT_STS_B\n");
    if ((memWrStsP0.tag == 0x0) && (memWrStsP0.okay == 1)) {
        if ((processed_bytes_rx) == 0) {
            if (!sImageLoaded.full()) {
                if (cnt_wr_img_loaded++ >= 1) {
                    sImageLoaded.write(false);
                    enqueueStrToDdrFSM = FSM_WR_PAT_STS_C;
                }
                else {
                    sImageLoaded.write(true);
                }
            }
        }
        else {
            enqueueStrToDdrFSM = FSM_WR_PAT_STS_C;
        }
    }
    else {
        ; // TODO: handle errors on memWrStsP0
    }
    break; 

case FSM_WR_PAT_STS_C:
    printf("DEBUG in pRXPathStreamToDDR: enqueueStrToDdrFSM - FSM_WR_PAT_STS_C\n");    
        if((processed_bytes_rx) == 0) {
            enqueueStrToDdrFSM = WAIT_FOR_META;
        }
        else {
            enqueueStrToDdrFSM = FSM_CHK_PROC_BYTES;
        }
    break;
}

}

