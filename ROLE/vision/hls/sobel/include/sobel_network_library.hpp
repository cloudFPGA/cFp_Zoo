/*****************************************************************************
 * @file       sobel_network_library.hpp
 * @brief      A library for some common Network-Related functionalities
 * @author     FAB, WEI, NGL, DID, DCO
 * @date       September 2021
 *----------------------------------------------------------------------------
 * @details      Implementations of library example Network-Related functionalities
 * Network-Related for
 *    setting the cluster port destination
 *    receiving(RX) some commands (generally data)
 *    receiving(RX) image and send to stream
 *    receiving(RX) streaming of data (image) and store it to ddr through stream interface (if)
 *    transmitting(TX) the test results back 
 * 
 * 
 * @deprecated
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup SobelHLS
 * @addtogroup SobelHLS
 * \{
 *****************************************************************************/

#ifndef _ROLE_SOBEL_LIBRARY_HPP_
#define _ROLE_SOBEL_LIBRARY_HPP_

#include <stdio.h>
#include <iostream>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>
#include "../../../../../HOST/vision/sobel/languages/cplusplus/include/config.h"//debug level define 
#include "memory_utils.hpp" //for stream based communication with ddr
#include "network.hpp"

using namespace hls;

#define FSM_WRITE_NEW_DATA 0
#define FSM_DONE 1
#define PortFsmType uint8_t
// Starting with 2718, this number corresponds to the extra opened ports of this role. Every bit set
// corresponds to one port.
// e.g. 0x1->2718, 0x2->2719, 0x3->[2718,2719], 0x7->[2718,2719,2720], 0x17->[2718-2722], etc.
#define PORTS_OPENED 0x1F

#ifdef ENABLE_DDR
#if TRANSFERS_PER_CHUNK_DIVEND == 0
#define TRANSFERS_PER_CHUNK_LAST_BURST TRANSFERS_PER_CHUNK
#else
#define TRANSFERS_PER_CHUNK_LAST_BURST TRANSFERS_PER_CHUNK_DIVEND
#endif
#endif


//////////////////////////////////////////////////////////////////////////////
//////////////////Begin of Network-Related Functions//////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * @brief pPortAndDestionation - Setup the port and the destination rank.
 *
 * @param[in]  pi_rank
 * @param[in]  pi_size
 * @param[out] sDstNode_sig
 * @param[out] po_rx_ports
 *
 * @return Nothing.
 ******************************************************************************/
void pPortAndDestionation(
    ap_uint<32>             *pi_rank,
    ap_uint<32>             *pi_size,
    stream<NodeId>          &sDstNode_sig,
    ap_uint<32>             *po_rx_ports
    )
{
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS INLINE off
  //-- STATIC VARIABLES (with RESET) ------------------------------------------
  static PortFsmType port_fsm = FSM_WRITE_NEW_DATA;
#pragma HLS reset variable=port_fsm

  switch(port_fsm)
  {
    default:
    case FSM_WRITE_NEW_DATA:
        printf("DEBUG in pPortAndDestionation: port_fsm - FSM_WRITE_NEW_DATA\n");       
        //Sobel app needs to be reset to process new rank
        if(!sDstNode_sig.full())
        {
          NodeId dst_rank = (*pi_rank + 1) % *pi_size;
    #if DEBUG_LEVEL == TRACE_ALL
          printf("rank: %d; size: %d; \n", (int) *pi_rank, (int) *pi_size);
    #endif
          sDstNode_sig.write(dst_rank);
          *po_rx_ports = 0x0; //init the value
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
 * @brief Receive Path - From SHELL to THIS.
 * FIXME: never checked, just substitute this one from DID
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
    //stream<Data_t_in>                   &img_in_axi_stream,
    stream<ap_uint<INPUT_PTR_WIDTH>>      &img_in_axi_stream,    
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
    static PacketFsmType enqueueFSM = WAIT_FOR_META;
    #pragma HLS reset variable=enqueueFSM

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
 * @brief Receive Path - From SHELL to THIS. Function for accumulating a memory word and write it
 *  Not ready for complete parametrization
 *
 * @param[in]  siSHL_This_Data the data rx from network
 * @param[in]  siNrc_meta meta from network
 * @param[out] sRxtoTx_Meta meta to the tx  path
 * @param[out] img_in_axi_stream the image data packed in 512 bits
 * @param[out] sMemBurstRx assessing the burst is ready
 *
 * @return Nothing.
 ******************************************************************************/
template<typename TMemWrd, const unsigned int  loop_cnt, const unsigned int cTransfers_Per_Chunk>
void pRXPathNetToStream(
    stream<NetworkWord>                 &siSHL_This_Data,
    stream<NetworkMetaStream>           &siNrc_meta,
    stream<NetworkMetaStream>           &sRxtoTx_Meta,
    stream<ap_uint<TMemWrd>>            &img_in_axi_stream,
    stream<bool>                        &sMemBurstRx
    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS INLINE off
    
    //-- LOCAL VARIABLES ------------------------------------------------------
    static NetworkWord  netWord;
    // const unsigned int  loop_cnt = (MEMDW_512/BITS_PER_10GBITETHRNET_AXI_PACKET);
    NetworkMetaStream   meta_tmp;
    static ap_uint<TMemWrd> v = 0;
    static unsigned int cnt_wr_stream = 0, cnt_wr_burst = 0;      
    #pragma HLS reset variable=cnt_wr_stream
    #pragma HLS reset variable=cnt_wr_burst
    static PacketFsmType enqueueRxToStrFSM = WAIT_FOR_META;
    #pragma HLS reset variable=enqueueRxToStrFSM

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
            if ((netWord.tkeep >> cnt_wr_stream) == 0) {
                printf("WARNING: value with tkeep=0 at cnt_wr_stream=%u\n", cnt_wr_stream);
            }
            v(cnt_wr_stream*64, (cnt_wr_stream+1)*64-1) = netWord.tdata(0,63);
            if ((cnt_wr_stream++ == loop_cnt-1) || (netWord.tlast == 1)) {
                std::cout << "DEBUG in pRXPathNetToStream: Pushing to img_in_axi_stream :" << std::hex << v << std::endl;
                img_in_axi_stream.write(v);
                if ((cnt_wr_burst++ == cTransfers_Per_Chunk-1) || (netWord.tlast == 1)) {
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
 * @brief Receive Path - From RX path stream word aligned to store towards the DDR
 *
 * @param[in]  img_in_axi_stream
 * @param[in]  sMemBurstRx
 * @param[out] soMemWrCmdP0
 * @param[out] siMemWrStsP0
 * @param[out] soMemWriteP0
 * @param[out] sImageLoaded
 *
 * @return Nothing.
 ******************************************************************************/
template <typename TMemWrd,const unsigned int loop_cnt,const unsigned int bytes_per_loop>
void pRXPathStreamToDDR(
    stream<ap_uint<TMemWrd>>          &img_in_axi_stream,
    stream<bool>                      &sMemBurstRx,    
    //---- P0 Write Path (S2MM) -----------
    stream<DmCmd>                     &soMemWrCmdP0,
    stream<DmSts>                     &siMemWrStsP0,
    stream<Axis<TMemWrd> >            &soMemWriteP0,
    //---- P1 Memory mapped ---------------
    stream<bool>                      &sImageLoaded
    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS INLINE off
    #pragma HLS pipeline II=1
    
    //-- LOCAL VARIABLES ------------------------------------------------------
    static ap_uint<TMemWrd> v = 0;
    // const unsigned int loop_cnt = (MEMDW_512/BITS_PER_10GBITETHRNET_AXI_PACKET);
    // const unsigned int bytes_per_loop = (BYTES_PER_10GBITETHRNET_AXI_PACKET*loop_cnt);
    static unsigned int cur_transfers_per_chunk;
    static unsigned int cnt_wr_stream, cnt_wr_img_loaded;
    static unsigned int ddr_addr_in; 
    static PacketFsmType enqueueStrToDdrFSM = WAIT_FOR_META;
    #pragma HLS reset variable=enqueueStrToDdrFSM

    static ap_uint<32> patternWriteNum;
    static ap_uint<32> timeoutCnt;
    
    static Axis<TMemWrd>     memP0;
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



//TODO:
// void pTXPath(
//         stream<NodeId>              &sDstNode_sig,
//         stream<NetworkWord>         &soTHIS_Shl_Data,
//         stream<NetworkMetaStream>   &soNrc_meta,
//         stream<NetworkWord>         &sRxpToTxp_Data,
//         stream<NetworkMetaStream>   &sRxtoTx_Meta,
//         unsigned int                *processed_word_tx,
//         ap_uint<32>                 *pi_rank,
//         ap_uint<32>                 *pi_size
//         )   
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
    #pragma  HLS INLINE off

    //-- STATIC DATAFLOW VARIABLES ------------------------------------------
    static NodeId dst_rank;
    static PacketFsmType dequeueFSM = WAIT_FOR_META;
    #pragma HLS reset variable=dequeueFSM
    #pragma HLS reset variable=dst_rank

    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord      netWordTx;
    NetworkMeta  meta_in = NetworkMeta();
    #pragma HLS reset variable=netWordTx
  
  switch(dequeueFSM)
  {
    default:
    case WAIT_FOR_META:
      if(!sDstNode_sig.empty())
      {
        dst_rank = sDstNode_sig.read();
        dequeueFSM = WAIT_FOR_STREAM_PAIR;
        //Sobel app needs to be reset to process new rank
      }
      break;
    case WAIT_FOR_STREAM_PAIR:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG in pTXPath: dequeueFSM=%d - WAIT_FOR_STREAM_PAIR, *processed_word_tx=%u\n", 
       dequeueFSM, *processed_word_tx);
    #endif
      //-- Forward incoming chunk to SHELL
      // *processed_word_tx = 0;
      //Sobel-related
      if (*processed_word_tx == MIN_TX_LOOPS) {
        *processed_word_tx = 0;
      }

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

        //Sobel-related Forcing the SHELL to wait for tlast
        meta_out_stream.tdata.len = 0;

        meta_out_stream.tdata.dst_rank = dst_rank;
        meta_out_stream.tdata.src_rank = (NodeId) *pi_rank;
        meta_out_stream.tdata.dst_port = meta_in.src_port;
        meta_out_stream.tdata.src_port = meta_in.dst_port;

        soNrc_meta.write(meta_out_stream);

        (*processed_word_tx)++;
	      printf("DEBUG: Checking netWordTx.tlast...\n");
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
            printf("DEBUG: A netWordTx.tlast=1 ... sRxpToTxp_Data.empty()==%u \n", sRxpToTxp_Data.empty());
            dequeueFSM = WAIT_FOR_STREAM_PAIR;
        }
        
        soTHIS_Shl_Data.write(netWordTx);
      }
      break;
  }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////End of Network-Related Functions////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#endif //_ROLE_SOBEL_LIBRARY_HPP_