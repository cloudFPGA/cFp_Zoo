/*****************************************************************************
 * @file       sobel_processing.hpp
 * @brief      The processing function for the sobel filte
 * @author     DCO
 * @date       September 2021
 *----------------------------------------------------------------------------
 *
 * @details      A Sobel Filter FSM for the processing of data
 *
 * @deprecated   
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup SobelHLS
 * @addtogroup SobelHLS
 * \{
 *****************************************************************************/  
#ifndef _ROLE_SOBEL_PROCESSING_HPP_
#define _ROLE_SOBEL_PROCESSING_HPP_


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




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////This function represent the main custom logic. it receives data form ///
///// the RX and transmit what needed to the TX //////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///////////////////// CUSTOM LOGIC INSERTION HERE ////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 * @brief THIS processing the data once recieved a start command 
 * Template function for custom processing
 *
 * @param[in]  sRxpToProcp_Data stream of data from rx to proc interface
 * @param[out] sProcpToTxp_Data stream of data from proc to tx interface
 * @param[in]  start_stop start and stop command
 * @param[in]  lcl_mem0 shell-role mp1 memory mapped interfce virtual ptr 0
 * @param[in]  lcl_mem1 shell-role mp1 memory mapped interfce virtual ptr 1
 *
 * @return Nothing.
 ******************************************************************************/
 template<const unsigned int counter_width=64>
 void pTHISProcessingData(
  stream<NetworkWord>                              &sRxpToProcp_Data,
  stream<NetworkWord>                              &sProcpToTxp_Data,
  stream<NetworkMetaStream>                        &sRxtoProc_Meta,
  stream<NetworkMetaStream>                        &sProctoTx_Meta,
  bool *                                           start_stop//,
  #ifdef ENABLE_DDR
                                                                    ,
  //------------------------------------------------------
  //-- SHELL / Role / Mem / Mp1 Interface
  //------------------------------------------------------    
  membus_t                    *lcl_mem0,
  membus_t                    *lcl_mem1
  #endif
){
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma  HLS INLINE off
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord    netWord;
    NetworkWord    outNetWord;
    static NetworkMetaStream  outNetMeta = NetworkMetaStream();;
    static ProcessingFsmType processingFSM  = FSM_PROCESSING_WAIT_FOR_META;

    static ap_uint<16> max_iterations;

    static local_mem_addr_t first_faulty_address;

    static ap_uint<32> faulty_addresses_cntr;

    static local_mem_addr_t max_address_under_test; // byte addressable;
    static size_t bytes_sent_for_tx =0;

#pragma HLS reset variable=processingFSM
#pragma HLS reset variable=outNetMeta
#pragma HLS reset variable=max_iterations
#pragma HLS reset variable=first_faulty_address
#pragma HLS reset variable=faulty_addresses_cntr
#pragma HLS reset variable=max_address_under_test
#pragma HLS reset variable=processingFSM
#pragma HLS reset variable=bytes_sent_for_tx

    static ap_uint<32> testCounter;
    static ap_uint<counter_width> reading_cntr = 0;
    static ap_uint<counter_width> writing_cntr = 0;
    static unsigned int burst_size=1;
    static local_mem_addr_t tmp_wordaligned_address = 0;
    static int emptycntr=0;

#pragma HLS reset variable=testCounter
#pragma HLS reset variable=tmp_wordaligned_address
#pragma HLS reset variable=burst_size


//assuming that whnever I send a start I must complete the run and then restart unless a stop
// or stopping once done with the run iterations
    switch(processingFSM)
    {
      case FSM_PROCESSING_WAIT_FOR_META:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG proc FSM, I am in the WAIT_FOR_META state\n");
    #endif
      //resetting once per test suite
      max_address_under_test = 0;
      max_iterations=0;
      bytes_sent_for_tx = 0;
      burst_size=1;
      if ( !sRxtoProc_Meta.empty()  && !sProctoTx_Meta.full())
      {
        outNetMeta = sRxtoProc_Meta.read();
        sProctoTx_Meta.write(outNetMeta);
        processingFSM = FSM_PROCESSING_PCKT_PROC;
      }
      break;

      case FSM_PROCESSING_PCKT_PROC:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG proc FSM, I am in the PROCESSING_PCKT_PROC state\n");
    #endif
//parse the received data
      if ( !sRxpToProcp_Data.empty() && !sProcpToTxp_Data.full())
      {
        netWord = sRxpToProcp_Data.read();
        switch (netWord.tdata.range(MEMTEST_COMMANDS_HIGH_BIT,MEMTEST_COMMANDS_LOW_BIT))
        {
        case TEST_INVLD_CMD:
          /* FWD an echo of the invalid*/
    #if DEBUG_LEVEL == TRACE_ALL
          printf("DEBUG processing the packet with invalid cmd\n");
    #endif
          sProcpToTxp_Data.write(netWord);
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;
        case TEST_STOP_CMD:
          /* call with stop (never started), unset, fwd the stop */
    #if DEBUG_LEVEL == TRACE_ALL
         printf("DEBUG processing the packet with stop cmd\n");
    #endif
          outNetWord.tdata=TEST_STOP_CMD;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 1;
          sProcpToTxp_Data.write(outNetWord);
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;    
        default:
          /* Execute the test if not invalid or stop*/
    #if DEBUG_LEVEL == TRACE_ALL
          printf("DEBUG processing the packet with the address within cmd\n");
    #endif
          max_address_under_test = netWord.tdata(MEMTEST_ADDRESS_HIGH_BIT,MEMTEST_ADDRESS_LOW_BIT);
          max_iterations = netWord.tdata.range(MEMTEST_ITERATIONS_HIGH_BIT,MEMTEST_ITERATIONS_LOW_BIT);
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
         printf("DEBUG processing the packet with the address %s within cmd %s\n", max_address_under_test.to_string().c_str(), max_iterations.to_string().c_str());
    #endif //__SYNTHESIS__
    #endif
          processingFSM = FSM_PROCESSING_BURST_READING;
          break;
        }
      }
      break;

      case FSM_PROCESSING_BURST_READING:
        #if DEBUG_LEVEL == TRACE_ALL
        printf("DEBUG proc FSM, I am in the FSM_PROCESSING_BURST_READING state\n");
       #endif
//parse the received data
      if ( !sRxpToProcp_Data.empty())
      {
        netWord = sRxpToProcp_Data.read();
    #if DEBUG_LEVEL == TRACE_ALL
        std::cout << netWord.tdata.to_string() << std::endl;
    #endif
        switch (netWord.tdata.range(MEMTEST_COMMANDS_HIGH_BIT,MEMTEST_COMMANDS_LOW_BIT))
        {
        case TEST_BURSTSIZE_CMD:
          /* extract the busrt size*/
          burst_size = netWord.tdata.range(MEMTEST_BURST_HIGH_BIT,MEMTEST_BURST_LOW_BIT);

    #if DEBUG_LEVEL == TRACE_ALL
          printf("DEBUG processing the packet with burst cmd, and burst size=%u\n",burst_size);
    #endif
          processingFSM = FSM_PROCESSING_START;
          break;


        default:
          /*unkown stuff hence using a burst with 1 beat*/
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
          printf("DEBUG processing the packet with smth bad within cmd: %s\n",netWord.tdata.range(MEMTEST_COMMANDS_HIGH_BIT,MEMTEST_COMMANDS_LOW_BIT).to_string().c_str());
    #endif
    #endif
          burst_size=1;
          sProcpToTxp_Data.write(netWord);
          processingFSM = FSM_PROCESSING_START;
          break;
        }
      }
      break;

//The hw can begin to do something
      case FSM_PROCESSING_START:
      // sOCMDPerfCounter.write(0);//init the counter
    #if DEBUG_LEVEL == TRACE_ALL
        printf("DEBUG proc FSM, I am in the START state\n");
    #endif
    //setting everything ready to start
        first_faulty_address = 0; 
        faulty_addresses_cntr = 0;
       if(max_address_under_test%64==0){
          tmp_wordaligned_address = max_address_under_test/64;
       } else {
         tmp_wordaligned_address = (max_address_under_test/64+1);
       }
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
        std::cout << " testing the address word aligned" << tmp_wordaligned_address.to_string() << std::endl;
    #endif
    #endif
        reading_cntr = 0;
        writing_cntr = 0;
        testCounter = 0;
        processingFSM = FSM_PROCESSING_DATAFLOW_WRITE;//FSM_PROCESSING_WRITE;
        break;

  //run continuously, hence more than once
      case FSM_PROCESSING_CONTINUOUS_RUN:
        testCounter += 1;
        reading_cntr = 0;
        writing_cntr = 0;
        faulty_addresses_cntr = 0;
        first_faulty_address = 0; 

        //stopping conditions: either a Stop or the maximum iterations
        if(*start_stop && testCounter < max_iterations){
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
          printf("DEBUG processing continuous run (still run, iter %s) max iters: %s\n",testCounter.to_string().c_str(),max_iterations.to_string().c_str());
    #endif //__SYNTHESIS__
    #endif
        // check if need another meta to send out!
        // if already over the MTU size, or with a command (stop case) or with another iteration I need to send out another meta
          if(bytes_sent_for_tx >= PACK_SIZE){
            sProctoTx_Meta.write(outNetMeta);
    #if DEBUG_LEVEL == TRACE_ALL
        std::cout <<  "DEBUG: writing an additional meta with bytes sent equal to " << bytes_sent_for_tx << std::endl;
    #endif
            bytes_sent_for_tx = 0;
          }
        processingFSM = FSM_PROCESSING_DATAFLOW_WRITE;

          break;
        }else{
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
          printf("DEBUG processing continuous run (stop the run at iter %s) max iters: %s \n",testCounter.to_string().c_str(),max_iterations.to_string().c_str());
    #endif //__SYNTHESIS__
    #endif
          //signal the end of the packet with the iteration of the tests performed
          outNetWord.tdata.range(MEMTEST_COMMANDS_HIGH_BIT,MEMTEST_COMMANDS_LOW_BIT)=TEST_ENDOFTESTS_CMD;
          outNetWord.tdata.range(NETWORK_WORD_BIT_WIDTH-1,MEMTEST_COMMANDS_HIGH_BIT+1)=testCounter;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 1;
          sProcpToTxp_Data.write(outNetWord);
          processingFSM = FSM_PROCESSING_WAIT_FOR_META;
          break;
        }

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////Custom User Processing Logic here///////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
      //Begin to process
      case FSM_PROCESSING_DATAFLOW_WRITE:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing write dataflow\n");
    #endif
    pWriteDataflowMemTest<top_param_maximum_number_of_beats>( lcl_mem0, 
    tmp_wordaligned_address , &writing_cntr,&testCounter,burst_size);
      processingFSM = FSM_PROCESSING_DATAFLOW_READ;
      break;

      case FSM_PROCESSING_WAIT_FOR_DDR_CONTROLLER_EMPTYNESS:
      #if DEBUG_LEVEL == TRACE_ALL
        printf("DEBUG processing the output of a run\n");
      #endif
        emptycntr++;
        if(emptycntr==DDR_LATENCY+1){
          processingFSM = FSM_PROCESSING_DATAFLOW_READ;
          emptycntr=0;
        }
      break;

      case FSM_PROCESSING_DATAFLOW_READ:
#if DEBUG_LEVEL == TRACE_ALL 
      printf("DEBUG processing read dataflow\n");
#endif
      pReadDataflowMemTest<top_param_maximum_number_of_beats>(lcl_mem1, 
      tmp_wordaligned_address ,&reading_cntr,&faulty_addresses_cntr, &first_faulty_address,burst_size);
      processingFSM = FSM_PROCESSING_OUTPUT;
      break;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////Custom output management Logic here/////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
      case FSM_PROCESSING_OUTPUT:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing the output of a run\n");
    #endif
      if(!sProcpToTxp_Data.full()){
          outNetWord.tdata = max_address_under_test;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          sProcpToTxp_Data.write(outNetWord);
          bytes_sent_for_tx += 8;
          processingFSM = FSM_PROCESSING_OUTPUT_2;
      }
      break;

      case FSM_PROCESSING_OUTPUT_2:
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
      printf("DEBUG processing the output of a run part 2; faulty address cntr %s\n",faulty_addresses_cntr.to_string().c_str());
    #endif
    #endif
      if(!sProcpToTxp_Data.full()){
          outNetWord.tdata=faulty_addresses_cntr;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          sProcpToTxp_Data.write(outNetWord);
          bytes_sent_for_tx += 8 ;
          processingFSM = FSM_PROCESSING_OUTPUT_3;
      }
      break;
      case FSM_PROCESSING_OUTPUT_3:
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
      printf("DEBUG processing the output of a run part 3: first faulty address %s\n",first_faulty_address.to_string().c_str());
    #endif
    #endif
      if(!sProcpToTxp_Data.full()){
          outNetWord.tdata=first_faulty_address;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          sProcpToTxp_Data.write(outNetWord);
          bytes_sent_for_tx += 8;
          processingFSM = FSM_PROCESSING_OUTPUT_4;
      }
      break;
      case FSM_PROCESSING_OUTPUT_4:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing the output of a run part 4\n");
    #endif
      if(!sProcpToTxp_Data.full()){
          outNetWord.tdata = writing_cntr;
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          sProcpToTxp_Data.write(outNetWord);
          bytes_sent_for_tx += 8;
          processingFSM = FSM_PROCESSING_OUTPUT_5;
      }
      break;
      case FSM_PROCESSING_OUTPUT_5:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing the output of a run part 4\n");
    #endif
      if(!sProcpToTxp_Data.full()){
          outNetWord.tdata= reading_cntr; 
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          sProcpToTxp_Data.write(outNetWord);
          bytes_sent_for_tx += 8;
          processingFSM = FSM_PROCESSING_CONTINUOUS_RUN;
      }
      break;

    }
};


        

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
        stream<ap_uint<INPUT_PTR_WIDTH>>        &img_in_axi_stream,
        stream<ap_uint<OUTPUT_PTR_WIDTH>>       &img_out_axi_stream,
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
    static PacketFsmType SobelFSM = WAIT_FOR_META;
    #pragma HLS reset variable=SobelFSM

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
#endif //_ROLE_SOBEL_PROCESSING_HPP_
