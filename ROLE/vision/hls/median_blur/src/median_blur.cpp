/*******************************************************************************
 * Copyright 2016 -- 2022 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*******************************************************************************/

/*****************************************************************************
 * @file       median_blur.cpp
 * @brief      The Role for a MedianBlur Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID
 * @date       Oct 2021
 *----------------------------------------------------------------------------
 *
 * @details      This application implements a UDP/TCP-oriented Vitis function.
 *
 * @deprecated   For the time being, we continue designing with the DEPRECATED
 *               directives because the new PRAGMAs do not work for us.
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup MedianBlurHLS
 * @addtogroup MedianBlurHLS
 * \{
 *****************************************************************************/
 
#include "../include/median_blur.hpp"
#include "../include/xf_median_blur_config.h"

#ifdef USE_HLSLIB_DATAFLOW
#include "../../../../../hlslib/include/hlslib/xilinx/Stream.h"
#include "../../../../../hlslib/include/hlslib/xilinx/Simulation.h"
#endif

#ifdef USE_HLSLIB_STREAM
using hlslib::Stream;
#endif
using hls::stream;

PacketFsmType  enqueueRxToStrFSM   = WAIT_FOR_META;
PacketFsmType  enqueueStrToDdrFSM  = WAIT_FOR_META;
PacketFsmType  enqueueFSM  = WAIT_FOR_META;
PacketFsmType  dequeueFSM  = WAIT_FOR_META;
PacketFsmType  MedianBlurFSM   = WAIT_FOR_META;

#ifdef ENABLE_DDR
#if TRANSFERS_PER_CHUNK_DIVEND == 0
#define TRANSFERS_PER_CHUNK_LAST_BURST TRANSFERS_PER_CHUNK
#else
#define TRANSFERS_PER_CHUNK_LAST_BURST TRANSFERS_PER_CHUNK_DIVEND
#endif
#endif

void pPortAndDestionation(
    ap_uint<32>             *pi_rank,
    ap_uint<32>             *pi_size,
    stream<NodeId>          &sDstNode_sig,
    ap_uint<32>             *po_rx_ports
    )
{
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS inline off
//#pragma HLS pipeline II=1 //not necessary
  //-- STATIC VARIABLES (with RESET) ------------------------------------------
  static PortFsmType port_fsm = FSM_WRITE_NEW_DATA;
#pragma HLS reset variable=port_fsm


  switch(port_fsm)
  {
    default:
    case FSM_WRITE_NEW_DATA:
        printf("DEBUG in pPortAndDestionation: port_fsm - FSM_WRITE_NEW_DATA\n");       
        //Triangle app needs to be reset to process new rank
        if(!sDstNode_sig.full())
        {
          NodeId dst_rank = (*pi_rank + 1) % *pi_size;
          printf("rank: %d; size: %d; \n", (int) *pi_rank, (int) *pi_size);
          sDstNode_sig.write(dst_rank);
          port_fsm = FSM_DONE;
        }
        break;
    case FSM_DONE:
        printf("DEBUG in pPortAndDestionation: port_fsm - FSM_DONE\n");        
        *po_rx_ports = PORTS_OPENED;
        break;
  }
}


 
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
  //stream<Data_t_in>                 &img_in_axi_stream,
  stream<ap_uint<INPUT_PTR_WIDTH>>    &img_in_axi_stream,
  #endif
  unsigned int                      *processed_word_rx,
  unsigned int                      *processed_bytes_rx,
  stream<bool>                      &sImageLoaded    
)
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
    img_in_axi_stream.write(v.data);
    bytes_with_keep += bytes_per_loop;
  }
  /*
  if (*processed_word_rx < IMG_PACKETS-1) {
    (*processed_word_rx)++;
  }
  else {
    printf("DEBUG in storeWordToAxiStream: WARNING - you've reached the max depth of img. Will put *processed_word_rx = 0.\n");
    *processed_word_rx = 0;
  }*/
  if (*processed_bytes_rx < IMGSIZE-BYTES_PER_10GBITETHRNET_AXI_PACKET) {
    (*processed_bytes_rx) += bytes_with_keep;
    if (!sImageLoaded.full()) {
        sImageLoaded.write(false);
    }
  }
  else {
    printf("DEBUG in storeWordToAxiStream: WARNING - you've reached the max depth of img. Will put *processed_bytes_rx = 0.\n");
    *processed_bytes_rx = 0;
    if (!sImageLoaded.full()) {
        sImageLoaded.write(true);
    }
  }
}


#ifdef ENABLE_DDR

/*****************************************************************************
 * @brief Receive Path - From SHELL to THIS.
 *
 * @param[in]  siSHL_This_Data
 * @param[in]  siNrc_meta
 * @param[out] sRxtoTx_Meta
 * @param[out] img_in_axi_stream
 * @param[out] meta_tmp
 * @param[out] processed_word
 * @param[out] sImageLoaded
 *
 * @return Nothing.
 ******************************************************************************/
void pRXPathDDROLD(
    stream<NetworkWord>                 &siSHL_This_Data,
    stream<NetworkMetaStream>           &siNrc_meta,
    stream<NetworkMetaStream>           &sRxtoTx_Meta,
    //---- P0 Write Path (S2MM) -----------
    stream<DmCmd>                       &soMemWrCmdP0,
    stream<DmSts>                       &siMemWrStsP0,
    stream<Axis<MEMDW_512> >            &soMemWriteP0,
    //---- P1 Memory mapped ---------------
    NetworkMetaStream                   meta_tmp,
    unsigned int                        *processed_bytes_rx,
    stream<bool>                        &sImageLoaded
    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS INLINE off
    #pragma HLS pipeline II=1
    #pragma HLS interface ap_ctrl_none port=return    
    
    //-- LOCAL VARIABLES ------------------------------------------------------
    static NetworkWord    netWord;

    static ap_uint<MEMDW_512> v = 0;
    const unsigned int loop_cnt = (MEMDW_512/BITS_PER_10GBITETHRNET_AXI_PACKET);
    const unsigned int bytes_per_loop = (BYTES_PER_10GBITETHRNET_AXI_PACKET*loop_cnt);
    static unsigned int cur_transfers_per_chunk;
    static unsigned int cnt_wr_stream, cnt_wr_img_loaded;
    static stream<ap_uint<MEMDW_512>> img_in_axi_stream ("img_in_axi_stream");
    const unsigned int img_in_axi_stream_depth = TRANSFERS_PER_CHUNK; // the AXI burst size
    #pragma HLS stream variable=img_in_axi_stream depth=img_in_axi_stream_depth
    static unsigned int ddr_addr_in; 
  
    // FIXME: Initialize to zero
    static ap_uint<32> patternWriteNum;
    static ap_uint<32> timeoutCnt;
    
    static Axis<MEMDW_512>   memP0;
    static DmSts             memWrStsP0;    
   
    #pragma HLS reset variable=cur_transfers_per_chunk
    #pragma HLS reset variable=cnt_wr_stream
    #pragma HLS reset variable=cnt_wr_img_loaded    
    #pragma HLS reset variable=ddr_addr_in
    #pragma HLS reset variable=patternWriteNum
    #pragma HLS reset variable=timeoutCnt
    #pragma HLS reset variable=memP0
    #pragma HLS reset variable=memWrStsP0    
    
    switch(enqueueFSM)
    {
    case WAIT_FOR_META:
        printf("DEBUG in pRXPathDDR: enqueueFSM - WAIT_FOR_META, *processed_bytes_rx=%u\n",
               *processed_bytes_rx);
        
        printf("TOTMEMDW_512=%u\n", TOTMEMDW_512);
        printf("TRANSFERS_PER_CHUNK=%u\n", TRANSFERS_PER_CHUNK);
        printf("TRANSFERS_PER_CHUNK_DIVEND=%u\n", TRANSFERS_PER_CHUNK_DIVEND);
        printf("TRANSFERS_PER_CHUNK_LAST_BURST=%u\n", TRANSFERS_PER_CHUNK_LAST_BURST);
        //exit(-1);
        
        if ( !siNrc_meta.empty() && !sRxtoTx_Meta.full() )
        {
            meta_tmp = siNrc_meta.read();
            meta_tmp.tlast = 1; //just to be sure...
            sRxtoTx_Meta.write(meta_tmp);
            if ((*processed_bytes_rx) == 0) {
                memP0.tdata = 0;
                memP0.tlast = 0;
                memP0.tkeep = 0;
                patternWriteNum = 0;
                timeoutCnt = 0;
                cur_transfers_per_chunk = 0;                
                netWord.tlast = 0;
                netWord.tkeep = 0x0;
                netWord.tdata = 0x0;
                ddr_addr_in = 0;
                cnt_wr_stream = 0;
                v = 0;
                memWrStsP0.tag = 0;
                memWrStsP0.interr = 0;
                memWrStsP0.decerr = 0;
                memWrStsP0.slverr = 0;
                memWrStsP0.okay = 0;
            }
            enqueueFSM = PROCESSING_PACKET;
        }
        break;

    case PROCESSING_PACKET:
        printf("DEBUG in pRXPathDDR: enqueueFSM - PROCESSING_PACKET, *processed_bytes_rx=%u\n",
               *processed_bytes_rx);
        if ( !siSHL_This_Data.empty() )
        {
            //-- Read incoming data chunk
            netWord = siSHL_This_Data.read();
            printf("DEBUG in pRXPathDDR: Data write = {D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
               netWord.tdata.to_long(), netWord.tkeep.to_int(), netWord.tlast.to_int());            
            //enqueueFSM = LOAD_IN_STREAM;
            if ((netWord.tkeep >> cnt_wr_stream) == 0) {
                printf("WARNING: value with tkeep=0 at cnt_wr_stream=%u\n", cnt_wr_stream);
                //continue;
            }
            v(cnt_wr_stream*64, (cnt_wr_stream+1)*64-1) = netWord.tdata(0,63);
            if ((cnt_wr_stream++ == loop_cnt-1) || (netWord.tlast == 1)) {
                if ( !img_in_axi_stream.full() ) {
                    // std::cout << std::hex << v << std::endl; // print hexadecimal value
                    img_in_axi_stream.write(v);
                }
                enqueueFSM = FSM_CHK_PROC_BYTES;
                cnt_wr_stream = 0;
            }
        }
        break;

    case FSM_CHK_PROC_BYTES:
        printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_CHK_PROC_BYTES, processed_bytes_rx=%u\n", *processed_bytes_rx);
        if (*processed_bytes_rx < IMGSIZE-bytes_per_loop) {
            (*processed_bytes_rx) += bytes_per_loop;
        }
        else {
            printf("DEBUG in pRXPathDDR: WARNING - you've reached the max depth of img. Will put *processed_bytes_rx = 0.\n");
            *processed_bytes_rx = 0;
        }
        enqueueFSM = FSM_WR_PAT_CMD;
    break;

case FSM_WR_PAT_CMD:
    printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_WR_PAT_CMD\n");
    if ( !soMemWrCmdP0.full() ) {
        //-- Post a memory write command to SHELL/Mem/Mp0
        if (*processed_bytes_rx == 0){
            cur_transfers_per_chunk = TRANSFERS_PER_CHUNK_LAST_BURST;
        }
        else {
            cur_transfers_per_chunk = TRANSFERS_PER_CHUNK;
        }
        if (patternWriteNum == 0) { // Write cmd only the fitst time of every burst
            soMemWrCmdP0.write(DmCmd(ddr_addr_in * BPERMDW_512, cur_transfers_per_chunk*BPERMDW_512)); // Byte-addresable
        }
        ddr_addr_in++;
        enqueueFSM = FSM_WR_PAT_LOAD;
    }
    break;

case FSM_WR_PAT_LOAD:
    printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_WR_PAT_LOAD\n");
    // -- Assemble a 512-bit memory word with input values from stream
    if (patternWriteNum++ == cur_transfers_per_chunk - 1) {
        patternWriteNum = 0;
        enqueueFSM = FSM_WR_PAT_DATA;
    }
    else {
        if(netWord.tlast == 1) {
            enqueueFSM = WAIT_FOR_META;
        }
        else {
            enqueueFSM = PROCESSING_PACKET;
        }
    }
    break;
    
case FSM_WR_PAT_DATA:
    printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_WR_PAT_DATA\n");
    if (!soMemWriteP0.full()) {
        //-- Write a memory word to DRAM
        if (!img_in_axi_stream.empty()) {
            memP0.tdata = img_in_axi_stream.read();
        }
        ap_uint<8> keepVal = 0xFF;
        memP0.tkeep = (ap_uint<64>) (keepVal, keepVal, keepVal, keepVal, keepVal, keepVal, keepVal, keepVal);
        if (patternWriteNum++ == cur_transfers_per_chunk - 1) {
            printf("DEBUG: (patternWriteNum == cur_transfers_per_chunk -1) \n");
            memP0.tlast = 1;
            cnt_wr_img_loaded = 0;
            timeoutCnt = 0;
            patternWriteNum = 0;
            enqueueFSM = FSM_WR_PAT_STS_A;
        }
        else {
            memP0.tlast = 0;
        }
        soMemWriteP0.write(memP0);
    }
    break;

case FSM_WR_PAT_STS_A:
    printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_WR_PAT_STS_A\n");
    if (!siMemWrStsP0.empty()) {
        printf(" 1 \n");
        //-- Get the memory write status for Mem/Mp0
        siMemWrStsP0.read(memWrStsP0);
        enqueueFSM = FSM_WR_PAT_STS_B;
    }
    else {
        if (timeoutCnt++ >= CYCLES_UNTIL_TIMEOUT) {
            memWrStsP0.tag = 0;
            memWrStsP0.interr = 0;
            memWrStsP0.decerr = 0;
            memWrStsP0.slverr = 0;
            memWrStsP0.okay = 0;           
            enqueueFSM = FSM_WR_PAT_STS_B;
        }
    }
    break;

case FSM_WR_PAT_STS_B:
    printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_WR_PAT_STS_B\n");
    if ((memWrStsP0.tag == 0x0) && (memWrStsP0.okay == 1)) {
        if ((*processed_bytes_rx) == 0) {
            if (!sImageLoaded.full()) {
                if (cnt_wr_img_loaded++ >= 1) {
                    sImageLoaded.write(false);
                    enqueueFSM = FSM_WR_PAT_STS_C;
                }
                else {
                    sImageLoaded.write(true);
                }
            }
        }
        else {
            enqueueFSM = FSM_WR_PAT_STS_C;
        }
    }
    else {
        ; // TODO: handle errors on memWrStsP0
    }
    break; 

case FSM_WR_PAT_STS_C:
    printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_WR_PAT_STS_C\n");    
        if(netWord.tlast == 1) {
            enqueueFSM = WAIT_FOR_META;
        }
        else {
            enqueueFSM = PROCESSING_PACKET;
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
    static unsigned int processed_net_bytes_rx = 0;
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
        printf("DEBUG in pRXPathNetToStream: enqueueRxToStrFSM - PROCESSING_PACKET, processed_net_bytes_rx=%u\n", processed_net_bytes_rx);
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
                if ((cnt_wr_burst++ == TRANSFERS_PER_CHUNK-1) || 
                    ((processed_net_bytes_rx == IMGSIZE-BYTES_PER_10GBITETHRNET_AXI_PACKET) && 
                     (netWord.tlast == 1))) {
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
            if (processed_net_bytes_rx == IMGSIZE-BYTES_PER_10GBITETHRNET_AXI_PACKET) {                
                processed_net_bytes_rx = 0;
            }
            else {
                processed_net_bytes_rx += BYTES_PER_10GBITETHRNET_AXI_PACKET;            
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


#endif // ENABLE_DDR


/*****************************************************************************
 * @brief Receive Path - From SHELL to THIS.
 *
 * @param[in]  siSHL_This_Data
 * @param[in]  siNrc_meta
 * @param[out] sRxtoTx_Meta
 * @param[out] img_in_axi_stream
 * @param[out] meta_tmp
 * @param[out] processed_word
 * @param[out] sImageLoaded
 *
 * @return Nothing.
 ******************************************************************************/
void pRXPath(
    stream<NetworkWord>                 &siSHL_This_Data,
    stream<NetworkMetaStream>           &siNrc_meta,
    stream<NetworkMetaStream>           &sRxtoTx_Meta,
    #ifdef USE_HLSLIB_STREAM
    Stream<Data_t_in, MIN_RX_LOOPS>     &img_in_axi_stream,
    #else // !USE_HLSLIB_STREAM
    //stream<Data_t_in>                   &img_in_axi_stream,
    stream<ap_uint<INPUT_PTR_WIDTH>>      &img_in_axi_stream,    
    #endif // USE_HLSLIB_STREAM
    NetworkMetaStream                   meta_tmp,
    unsigned int                        *processed_word_rx,
    unsigned int                        *processed_bytes_rx,
    stream<bool>                        &sImageLoaded
    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS INLINE off
    #pragma HLS pipeline II=1
    
    //-- LOCAL VARIABLES ------------------------------------------------------
    static NetworkWord    netWord;

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
      break;

    case PROCESSING_PACKET:
        printf("DEBUG in pRXPath: enqueueFSM - PROCESSING_PACKET, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
        *processed_word_rx, *processed_bytes_rx);
        if ( !siSHL_This_Data.empty() && !img_in_axi_stream.full())
        {
            //-- Read incoming data chunk
            netWord = siSHL_This_Data.read();
            storeWordToAxiStream(netWord, img_in_axi_stream, processed_word_rx, processed_bytes_rx, 
                            sImageLoaded);
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
    
  switch(MedianBlurFSM)
  {
    case WAIT_FOR_META: 
      printf("DEBUG in pProcPath: WAIT_FOR_META\n");
      if (!sImageLoaded.empty())
        {
            if (sImageLoaded.read() == true) {
                MedianBlurFSM = PROCESSING_PACKET;
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
	    #ifdef FAKE_MedianBlur
            fakeMedianBlurAccelStream(img_in_axi_stream, img_out_axi_stream, MIN_RX_LOOPS, MIN_TX_LOOPS);
	    #else // !FAKE_MedianBlur
            medianBlurAccelStream(img_in_axi_stream, img_out_axi_stream, WIDTH, HEIGHT);
	    #endif // FAKE_MedianBlur
	    #endif // ENABLE_DDR
            accel_called = true;
            MedianBlurFSM = MEDIANBLUR_RETURN_RESULTS;
        }
        #ifndef ENABLE_DDR
        }
        #endif
    break;
      
    #ifdef ENABLE_DDR 
    case MEDIANBLUR_RETURN_RESULTS:
      printf("DEBUG in pProcPath: MEDIANBLUR_RETURN_RESULTS, ddr_addr_out=%u\n", ddr_addr_out);      
      if (accel_called == true) {
	
        printf("DEBUG in pProcPath: Accumulated %u net words (%u B) to complete a single DDR word\n", 
	       KWPERMDW_512, BPERMDW_512);
            tmp = lcl_mem1[ddr_addr_out];
            ddr_addr_out++;
            MedianBlurFSM = MEDIANBLUR_RETURN_RESULTS_ABSORB_DDR_LAT;
            timeoutCntAbs = 0;
      }
    break;
    
    case MEDIANBLUR_RETURN_RESULTS_ABSORB_DDR_LAT:
      printf("DEBUG in pProcPath: MEDIANBLUR_RETURN_RESULTS_ABSORB_DDR_LAT [%u out of %u]\n", timeoutCntAbs, DDR_LATENCY);        
        if (timeoutCntAbs++ == DDR_LATENCY) {
            MedianBlurFSM = MEDIANBLUR_RETURN_RESULTS_FWD; //MEDIANBLUR_RETURN_RESULTS_UNPACK;
            cnt_i = 0;
        }
    break;
    /*
    case MEDIANBLUR_RETURN_RESULTS_UNPACK:
      printf("DEBUG in pProcPath: MEDIANBLUR_RETURN_RESULTS_UNPACK, cnt_i=%u\n", cnt_i);        
        //for (unsigned int cnt_i=0; cnt_i<(MEMDW_512/OUTPUT_PTR_WIDTH); cnt_i++) {
            #if OUTPUT_PTR_WIDTH == 64
            raw64(0 ,63) = tmp(cnt_i*OUTPUT_PTR_WIDTH   , cnt_i*OUTPUT_PTR_WIDTH+63);
            #endif
            if ( !img_out_axi_stream.full() ) {
                img_out_axi_stream.write(raw64);
            }
            if (cnt_i == (MEMDW_512/OUTPUT_PTR_WIDTH) - 1) {
                MedianBlurFSM = MEDIANBLUR_RETURN_RESULTS_FWD;
            }
            cnt_i++;
        //}
    break;
    */
    case MEDIANBLUR_RETURN_RESULTS_FWD: 
      printf("DEBUG in pProcPath: MEDIANBLUR_RETURN_RESULTS_FWD\n");
      //if ( !img_out_axi_stream.empty() && !sRxpToTxp_Data.full() ) {
      if ( (cnt_i <= (MEMDW_512/OUTPUT_PTR_WIDTH) - 1) && !sRxpToTxp_Data.full() ) {
          
        //temp.data = img_out_axi_stream.read();
        temp.data(0 ,63) = tmp(cnt_i*OUTPUT_PTR_WIDTH   , cnt_i*OUTPUT_PTR_WIDTH+63);
        if (processed_word_proc++ == MIN_TX_LOOPS-1) {
            temp.last = 1;
            MedianBlurFSM = WAIT_FOR_META;
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
        MedianBlurFSM = MEDIANBLUR_RETURN_RESULTS;
      }
    
    break;

    #else // ! ENABLE_DDR
    case MEDIANBLUR_RETURN_RESULTS:
        printf("DEBUG in pProcPath: MEDIANBLUR_RETURN_RESULTS\n");
        if ( !img_out_axi_stream.empty() && !sRxpToTxp_Data.full() )
        {
	
            temp.data = img_out_axi_stream.read();
            if ( img_out_axi_stream.empty() )
            //if (processed_word_proc++ == MIN_TX_LOOPS-1)
            {
                temp.last = 1;
                MedianBlurFSM = WAIT_FOR_META;
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


unsigned int sRxpToTxp_DataCounter = 0;

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
        stream<NodeId>              &sDstNode_sig,
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
    #pragma HLS INLINE off
    #pragma HLS pipeline II=1
    
    //-- STATIC DATAFLOW VARIABLES ------------------------------------------
    static NodeId dst_rank;
    
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord netWordTx;
    NetworkMeta meta_in = NetworkMeta();
    NetworkMetaStream meta_out_stream = NetworkMetaStream();
    
    #pragma HLS reset variable=dst_rank
    #pragma HLS reset variable=netWordTx
    
  switch(dequeueFSM)
  {
    default:
    case WAIT_FOR_META:
      if(!sDstNode_sig.empty())
      {
        dst_rank = sDstNode_sig.read();
        dequeueFSM = WAIT_FOR_STREAM_PAIR;
        //MedianBlur app needs to be reset to process new rank
      }
      break;      
      
    case WAIT_FOR_STREAM_PAIR:
      printf("DEBUG in pTXPath: dequeueFSM=%d - WAIT_FOR_STREAM_PAIR, *processed_word_tx=%u\n", 
        dequeueFSM, *processed_word_tx);
      //-- Forward incoming chunk to SHELL
      if (*processed_word_tx == MIN_TX_LOOPS) {
        *processed_word_tx = 0;
      }
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
        //NetworkMetaStream meta_out_stream = NetworkMetaStream();
        meta_out_stream.tlast = 1;
        meta_out_stream.tkeep = 0xFF; //just to be sure

        meta_out_stream.tdata.dst_rank = dst_rank; //(*pi_rank + 1) % *pi_size;
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
	    printf("DEBUGGGG: Checking netWordTx.tlast...\n");
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
        printf("DEBUGGGG: Reading sRxpToTxp_Data %u\n", sRxpToTxp_DataCounter++);
        netWordTx = sRxpToTxp_Data.read();

        (*processed_word_tx)++;
        
        // This is a normal termination of the axi stream from vitis functions
        if ((netWordTx.tlast == 1) || (((*processed_word_tx)*8) % PACK_SIZE == 0))
        {
            netWordTx.tlast = 1; // in case it is the 2nd or
            printf("DEBUGGGG: A netWordTx.tlast=1 ... sRxpToTxp_Data.empty()==%u \n", sRxpToTxp_Data.empty());
            dequeueFSM = WAIT_FOR_STREAM_PAIR;
        }
	
        // This is our own termination based on the custom MTU we have set in PACK_SIZE.
        // TODO: We can map PACK_SIZE to a dynamically assigned value either through MMIO or header
        //       in order to have a functional bitstream for any MTU size
        //if (((*processed_word_tx)*8) % PACK_SIZE == 0) 
        //{
        //    printf("DEBUGGGG: B (*processed_word_tx)*8) % PACK_SIZE == 0 ...\n");
        //    netWordTx.tlast = 1;
        //    dequeueFSM = WAIT_FOR_STREAM_PAIR;
        //}
	
        soTHIS_Shl_Data.write(netWordTx);
      }
      break;
  }
}


/*****************************************************************************
 * @brief   Main process of the MedianBlur Application 
 * directives.
 * @deprecated  This functions is using deprecated AXI stream interface 
 * @return Nothing.
 *****************************************************************************/
void median_blur(

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

// Mapping LCL_MEM0 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem0 bundle=moMEM_Mp1\
  max_read_burst_length=max_axi_rw_burst_length  max_write_burst_length=max_axi_rw_burst_length offset=direct \
  num_read_outstanding=16 num_write_outstanding=16 latency=ddr_latency

// Mapping LCL_MEM1 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem1 bundle=moMEM_Mp1 \
  max_read_burst_length=max_axi_rw_burst_length  max_write_burst_length=max_axi_rw_burst_length offset=direct \
  num_read_outstanding=16 num_write_outstanding=16 latency=ddr_latency

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
#ifdef USE_HLSLIB_DATAFLOW
  static hlslib::Stream<Data_t_in,  MIN_RX_LOOPS> img_in_axi_stream ("img_in_axi_stream");
  static hlslib::Stream<Data_t_out, MIN_TX_LOOPS> img_out_axi_stream ("img_out_axi_stream");
#else
  static stream<ap_uint<INPUT_PTR_WIDTH>> img_in_axi_stream ("img_in_axi_stream");
  static stream<ap_uint<OUTPUT_PTR_WIDTH>> img_out_axi_stream ("img_out_axi_stream");
#endif
#endif
  //*po_rx_ports = 0x1; //currently work only with default ports...
  static stream<NodeId>            sDstNode_sig("sDstNode_sig");


//-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS stream variable=sRxtoTx_Meta depth=tot_transfers
#pragma HLS reset variable=enqueueFSM
#pragma HLS reset variable=dequeueFSM
#pragma HLS reset variable=MedianBlurFSM
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


#ifdef USE_HLSLIB_DATAFLOW
  /*! @copybrief median_blur()
   *  MedianBlur is enabled with hlslib support
   */
  /*! @copydoc median_blur()
   * Use this snippet to early check for C++ errors related to dataflow and bounded streams (empty 
   * and full) during simulation. It can also be both synthesized and used in co-simulation.
   * Practically we use hlslib when we want to run simulation as close as possible to the HW, by 
   * executing all functions of dataflow in thread-safe parallel executions, i.e the function 
   * HLSLIB_DATAFLOW_FINALIZE() acts as a barrier for the threads spawned to serve every function 
   * called in HLSLIB_DATAFLOW_FUNCTION(func, args...).
   */
   /*! @copydetails median_blur()
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
                //&image_loaded
                sImageLoaded
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
  
 pPortAndDestionation(
        pi_rank, 
        pi_size, 
        sDstNode_sig, 
        po_rx_ports
        );
  
#ifdef ENABLE_DDR
  /*
 pRXPathDDR(
        siSHL_This_Data,
        siNrc_meta,
        sRxtoTx_Meta,
        //---- P0 Write Path (S2MM) -----------
        soMemWrCmdP0,
        siMemWrStsP0,
        soMemWriteP0,
        // ---- P1 Memory mapped --------------
        meta_tmp,
        &processed_bytes_rx,
        sImageLoaded  
        );
 */
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
        sDstNode_sig,
        soTHIS_Shl_Data,
        soNrc_meta,
        sRxpToTxp_Data,
        sRxtoTx_Meta,
        &processed_word_tx,
        pi_rank,
        pi_size
        );
  
#endif // USE_HLSLIB_DATAFLOW
}


/*! \} */
