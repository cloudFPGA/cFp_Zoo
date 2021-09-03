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
PacketFsmType  dequeueFSM  = WAIT_FOR_META;
PacketFsmType  HarrisFSM   = WAIT_FOR_META;
fsmStateDDRdef fsmStateDDR = FSM_IDLE; //FSM_WR_PAT_CMD;


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
        *po_rx_ports = 0x1; //currently work only with default ports...
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
  stream<Data_t_in>                 &img_in_axi_stream,
  #endif
  unsigned int                      *processed_word_rx,
  unsigned int                      *processed_bytes_rx,
  //bool                              *image_loaded
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
    //*image_loaded = true;
    if (!sImageLoaded.full()) {
        sImageLoaded.write(true);
    }
  }
}


    

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
  //bool                      *image_loaded,
  stream<bool>              &sImageLoaded,
  bool                      *skip_read,
  bool                      *write_chunk_to_ddr_pending,
  //stream<bool>              &sWriteChunkToDdrPending,
  bool                      *ready_to_accept_new_data,
  bool                      *signal_init
)
{
    #pragma HLS INLINE off
    #pragma HLS pipeline II=1
    
    static NetworkWord    netWord;
    
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
  
  
  
    //printf("DEBUG: storeWordToMem, skip_read = %s, write_chunk_to_ddr_pending=%s, ready_to_accept_new_data=%s, *signal_init=%s, *image_loaded=%s\n", 
    //       *skip_read?"true":"false", *write_chunk_to_ddr_pending?"true":"false", 
    //       *ready_to_accept_new_data?"true":"false", *signal_init?"true":"false", 
    //       *image_loaded?"true":"false");
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
            if (sImageLoaded.empty()) {
                sImageLoaded.write(false);
            }            
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
                //if (sWriteChunkToDdrPending.empty()) {
                //    sWriteChunkToDdrPending.write(true);
                //}
                fsmStateDDR = FSM_WR_PAT_CMD;
            }
            else {
                *ready_to_accept_new_data = true;
                fsmStateDDR = FSM_CHK_SKIP;
            }
        break;
           
        case FSM_WR_PAT_CMD:
            printf("DEBUG in storeWordToMem: fsmStateDDR - FSM_WR_PAT_CMD\n");
            //if (!sWriteChunkToDdrPending.empty()) {
                //if ((sWriteChunkToDdrPending.read() == true) && !soMemWrCmdP0.full()) {
                    if (*write_chunk_to_ddr_pending && !soMemWrCmdP0.full()) {
                    //-- Post a memory write command to SHELL/Mem/Mp0
                    soMemWrCmdP0.write(DmCmd(((*ddr_addr_in)++) * BPERMDW_512, CHECK_CHUNK_SIZE)); // Byte-addresable
                    patternWriteNum = 0;
                    // -- Assemble a 512-bit memory word with input values from stream
                    for (unsigned int i=0; i<BPERMDW_512; i++) {
                        v = img_in_axi_stream.read();
                        tmp((i+1)*INPUT_PTR_WIDTH-1, i*INPUT_PTR_WIDTH ) = v.data;
                    }                    
                    fsmStateDDR = FSM_WR_PAT_DATA;
                //}
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
                    fsmStateDDR = FSM_WR_PAT_STS_A;
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
             
        case FSM_WR_PAT_STS_A:
            printf("DEBUG in storeWordToMem: fsmStateDDR - FSM_WR_PAT_STS_A\n");                
            if (!siMemWrStsP0.empty()) {
                printf(" 1 \n");
                //-- Get the memory write status for Mem/Mp0
                siMemWrStsP0.read(memWrStsP0);
                // TODO: handle errors on memWrStsP0
                fsmStateDDR = FSM_IDLE;
                *write_chunk_to_ddr_pending = false; // exit from loop
                //if (sWriteChunkToDdrPending.empty()) {
                //    sWriteChunkToDdrPending.write(false);
                //}                
                if ((*processed_bytes_rx) == 0) {
                    //*image_loaded = true;
                    if (sImageLoaded.empty()) {
                        sImageLoaded.write(true);
                    }
                }
            }
            else {
                printf(" 2 \n");
                timeoutCnt++;
                if (timeoutCnt >= CYCLES_UNTIL_TIMEOUT) {
                    printf(" 3 \n");
                    fsmStateDDR = FSM_IDLE;
                    *write_chunk_to_ddr_pending = false; // exit from loop but with an error
                    //if (sWriteChunkToDdrPending.empty()) {
                    //    sWriteChunkToDdrPending.write(false);
                    //}
                    if ((*processed_bytes_rx) == 0) {
                        //*image_loaded = true;
                        if (sImageLoaded.empty()) {
                            sImageLoaded.write(true);
                        }
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
void pRXPathDDR(
    stream<NetworkWord>                 &siSHL_This_Data,
    stream<NetworkMetaStream>           &siNrc_meta,
    stream<NetworkMetaStream>           &sRxtoTx_Meta,
    //---- P0 Write Path (S2MM) -----------
    stream<DmCmd>                       &soMemWrCmdP0,
    stream<DmSts>                       &siMemWrStsP0,
    stream<Axis<MEMDW_512> >            &soMemWriteP0,
    //---- P1 Memory mapped ---------------
    membus_t                            *lcl_mem0,
    NetworkMetaStream                   meta_tmp,
    unsigned int                        *processed_word_rx,
    unsigned int                        *processed_word_tx,
    unsigned int                        *processed_bytes_rx,
    //bool                                *image_loaded
    stream<bool>                        &sImageLoaded
    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS INLINE off
    #pragma HLS pipeline II=1
    
    //-- LOCAL VARIABLES ------------------------------------------------------
    static NetworkWord    netWord;

    ap_uint<INPUT_PTR_WIDTH> v = 0;
    //v.data = 0;
    //v.keep = 0;
    //v.last = 0;
    const unsigned int loop_cnt = (BITS_PER_10GBITETHRNET_AXI_PACKET/INPUT_PTR_WIDTH);
    const unsigned int bytes_per_loop = (BYTES_PER_10GBITETHRNET_AXI_PACKET/loop_cnt);
    static unsigned int bytes_with_keep;
    static unsigned int cnt_rd_stream, cnt_wr_stream, cnt_wr_img_loaded;
    //static stream<Data_t_in> img_in_axi_stream ("img_in_axi_stream");
    static stream<ap_uint<INPUT_PTR_WIDTH>> img_in_axi_stream ("img_in_axi_stream");
    #pragma HLS stream variable=img_in_axi_stream depth=65
    // reuse the unused register 'processed_word_rx' for 'ddr_addr_in'
    static unsigned int ddr_addr_in; //= processed_word_rx;
    static membus_t tmp;
  
    // FIXME: Initialize to zero
    static ap_uint<32> patternWriteNum;
    static ap_uint<32> timeoutCnt;
    
    Axis<MEMDW_512>   memP0;
    DmSts             memRdStsP0;
    DmSts             memWrStsP0;    
   
    #pragma HLS reset variable=bytes_with_keep
    #pragma HLS reset variable=cnt_rd_stream
    #pragma HLS reset variable=cnt_wr_stream
    #pragma HLS reset variable=cnt_wr_img_loaded    
    #pragma HLS reset variable=ddr_addr_in
    #pragma HLS reset variable=tmp
    #pragma HLS reset variable=patternWriteNum
    #pragma HLS reset variable=timeoutCnt
    #pragma HLS reset variable=memP0
    #pragma HLS reset variable=memRdStsP0
    #pragma HLS reset variable=memWrStsP0    
    
    switch(enqueueFSM)
    {
    case WAIT_FOR_META:
        printf("DEBUG in pRXPathDDR: enqueueFSM - WAIT_FOR_META, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
               *processed_word_rx, *processed_bytes_rx);
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
                tmp = 0;
                bytes_with_keep = 0;                
                netWord.tlast = 0;
                netWord.tkeep = 0x0;
                netWord.tdata = 0x0;
                ddr_addr_in = 0;
                cnt_rd_stream = 0;
                cnt_wr_stream = 0;
            }
            enqueueFSM = PROCESSING_PACKET;
        }
        //*image_loaded = false;
        //if (sImageLoaded.empty()) {
        //    sImageLoaded.write(false);
        //}
        break;

    case PROCESSING_PACKET:
        printf("DEBUG in pRXPathDDR: enqueueFSM - PROCESSING_PACKET, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
               *processed_word_rx, *processed_bytes_rx);
        if ( !siSHL_This_Data.empty() )
        {
            //-- Read incoming data chunk
            netWord = siSHL_This_Data.read();
            cnt_wr_stream = 0;
            enqueueFSM = LOAD_IN_STREAM;
        }
        break;


    case LOAD_IN_STREAM:
        printf("DEBUG in pRXPathDDR: enqueueFSM - LOAD_IN_STREAM\n");
        printf("DEBUG in pRXPathDDR: Data write = {D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
               netWord.tdata.to_long(), netWord.tkeep.to_int(), netWord.tlast.to_int());
        //printf("DEBUG: Checking: netWord.tkeep=%u >> %u = %u\n", netWord.tkeep.to_int(), cnt_wr_stream, (netWord.tkeep.to_int() >> cnt_wr_stream));
        if ((netWord.tkeep >> cnt_wr_stream) == 0) {
            printf("WARNING: value with tkeep=0 at cnt_wr_stream=%u\n", cnt_wr_stream);
            //continue;
        }
        v = (ap_uint<INPUT_PTR_WIDTH>)(netWord.tdata >> cnt_wr_stream*8);
        //v.keep = netWord.tkeep;
        //v.last = netWord.tlast;
        if ( !img_in_axi_stream.full() ) {
            img_in_axi_stream.write(v);
        }
        bytes_with_keep += bytes_per_loop;

        if (cnt_wr_stream++ == loop_cnt-1) {
            enqueueFSM = FSM_CHK_PROC_BYTES;
        }
    break;
    

    case FSM_CHK_PROC_BYTES:
        printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_CHK_PROC_BYTES, processed_bytes_rx=%u\n", *processed_bytes_rx);
        if (*processed_bytes_rx < IMGSIZE-BYTES_PER_10GBITETHRNET_AXI_PACKET) {
            (*processed_bytes_rx) += bytes_with_keep;
        }
        else {
            printf("DEBUG in storeWordToMem: WARNING - you've reached the max depth of img. Will put *processed_bytes_rx = 0.\n");
            *processed_bytes_rx = 0;
        }
        bytes_with_keep = 0;
        enqueueFSM = FSM_CHK_WRT_CHNK_TO_DDR_PND;
    break;

    case FSM_CHK_WRT_CHNK_TO_DDR_PND :
        printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_CHK_WRT_CHNK_TO_DDR_PND\n");

        // Both when we have a new word for DDR or the net stream ended (*processed_bytes_rx = 0)
        if ((*processed_bytes_rx) % BPERMDW_512 == 0) {
            printf("DEBUG in storeWordToMem: Accumulated %u net words (%u B) to complete a single DDR word\n",
               KWPERMDW_512, BPERMDW_512);
            enqueueFSM = FSM_WR_PAT_CMD;
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

case FSM_WR_PAT_CMD:
    printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_WR_PAT_CMD\n");
    if ( !soMemWrCmdP0.full() ) {
        //-- Post a memory write command to SHELL/Mem/Mp0
        soMemWrCmdP0.write(DmCmd(ddr_addr_in * BPERMDW_512, CHECK_CHUNK_SIZE)); // Byte-addresable
        patternWriteNum = 0;
        ddr_addr_in++;
        cnt_rd_stream = 0;
        tmp = 0;
        enqueueFSM = FSM_WR_PAT_LOAD;
    }
    break;

case FSM_WR_PAT_LOAD:
    printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_WR_PAT_LOAD\n");
    // -- Assemble a 512-bit memory word with input values from stream
    if ( !img_in_axi_stream.empty() ) {
        v = img_in_axi_stream.read();
    }
    tmp((cnt_rd_stream+1)*INPUT_PTR_WIDTH-1, cnt_rd_stream*INPUT_PTR_WIDTH ) = v;

    if (cnt_rd_stream++ == BPERMDW_512-1) {
        enqueueFSM = FSM_WR_PAT_DATA;
    }
    break;
    
case FSM_WR_PAT_DATA:
    printf("DEBUG in pRXPathDDR: enqueueFSM - FSM_WR_PAT_DATA\n");
    if (!soMemWriteP0.full()) {
        //-- Write a memory word to DRAM
        memP0.tdata = tmp; // (ap_uint<512>) (currentMemPattern,currentMemPattern,currentMemPattern,currentMemPattern,currentMemPattern,currentMemPattern,currentMemPattern,currentMemPattern);
        ap_uint<8> keepVal = 0xFF;
        memP0.tkeep = (ap_uint<64>) (keepVal, keepVal, keepVal, keepVal, keepVal, keepVal, keepVal, keepVal);
        if(patternWriteNum == TRANSFERS_PER_CHUNK -1) {
            printf("DEBUG: (patternWriteNum == TRANSFERS_PER_CHUNK -1) \n");
            memP0.tlast = 1;
            cnt_wr_img_loaded = 0;
            enqueueFSM = FSM_WR_PAT_STS_A;
        }
        else {
            memP0.tlast = 0;
        }
        soMemWriteP0.write(memP0);
        patternWriteNum++;
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
    if ((memWrStsP0.tag = 7) && (memWrStsP0.okay = 1)) {
        if ((*processed_bytes_rx) == 0) {
            if (!sImageLoaded.full()) {
                if (cnt_wr_img_loaded++ >= 1) {
                    sImageLoaded.write(false);
                    enqueueFSM = FSM_WR_PAT_STS_C;
                }
                else {
                    //*image_loaded = true;
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
/*
case WAIT_FOR_TX:
    printf("DEBUG in pRXPathDDR: enqueueFSM - WAIT_FOR_TX, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
           *processed_word_rx, *processed_bytes_rx);
    //printf("DEBUG: MIN_TX_LOOPS-1=%u", MIN_TX_LOOPS-1); exit(0);
    if (*processed_word_tx == MIN_TX_LOOPS) {
        enqueueFSM = WAIT_FOR_META;
    }
    break;
 */
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
    unsigned int                        *processed_word_tx,
    unsigned int                        *processed_bytes_rx,
    //bool                                *image_loaded,
    stream<bool>                        &sImageLoaded,
    bool                                *skip_read,
    bool                                *write_chunk_to_ddr_pending,
    //stream<bool>                        &sWriteChunkToDdrPending,    
    bool                                *ready_to_accept_new_data,
    bool                                *signal_init
    )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    #pragma HLS INLINE off
    #pragma HLS pipeline II=1
    
    //-- LOCAL VARIABLES ------------------------------------------------------
    static NetworkWord    netWord;
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
                //if (sWriteChunkToDdrPending.empty()) {
                //    sWriteChunkToDdrPending.write(false);
                //}                   
                *ready_to_accept_new_data = false;
                *signal_init = true;
                netWord.tlast = 0;
                netWord.tkeep = 0x0;
                netWord.tdata = 0x0;
            }
            #endif        
            enqueueFSM = PROCESSING_PACKET;
        }
#ifndef ENABLE_DDR
        //*image_loaded = false;
        if (!sImageLoaded.full()) {
            sImageLoaded.write(false);
        }
#endif
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
            #endif
            )
        {
            #ifdef ENABLE_DDR 
            //if (!sWriteChunkToDdrPending.empty()) {
            if ((*write_chunk_to_ddr_pending == false) && 
                //if ((sWriteChunkToDdrPending.read() == false) &&
                    !siSHL_This_Data.empty() &&
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
            if ((*write_chunk_to_ddr_pending == false) && (*ready_to_accept_new_data == true)) {
                    if ((*processed_bytes_rx) != 0) {
                        *signal_init = true;
                    }
                    if (netWord.tlast == 1) {
                        if ((*processed_bytes_rx) < (IMGSIZE-BYTES_PER_10GBITETHRNET_AXI_PACKET)) {
                            enqueueFSM = WAIT_FOR_META;
                        }
                        else {
                            enqueueFSM = WAIT_FOR_TX;
                            //*image_loaded = false;
                            //*skip_read = false;                        
                        }
                    }
                }
            //}
            
            #else // ! ENABLE_DDR
            
            //-- Read incoming data chunk
            netWord = siSHL_This_Data.read();
            storeWordToAxiStream(netWord, img_in_axi_stream, processed_word_rx, processed_bytes_rx, 
                            sImageLoaded);
            if(netWord.tlast == 1)
            {
                enqueueFSM = WAIT_FOR_META;
            }            
            
            #endif // ENABLE_DDR
        }
      break;
      
    #ifdef ENABLE_DDR 
    case WAIT_FOR_TX:
        printf("DEBUG in pRXPath: enqueueFSM - WAIT_FOR_TX, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
                *processed_word_rx, *processed_bytes_rx);
       //printf("DEBUG: MIN_TX_LOOPS-1=%u", MIN_TX_LOOPS-1); exit(0);
        if (*processed_word_tx == MIN_TX_LOOPS) {
            enqueueFSM = WAIT_FOR_META;
        }
      
      break;  
    #endif // ENABLE_DDR
      
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
        //bool                                    *image_loaded
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
    static stream<Data_t_out> img_out_axi_stream ("img_out_axi_stream");
    #pragma HLS stream variable=img_out_axi_stream depth=9
    static unsigned int ddr_addr_out;
    #endif
    
    #pragma HLS reset variable=accel_called  
    #pragma HLS reset variable=processed_word_proc  
    #pragma HLS reset variable=timeoutCntAbs  
    #pragma HLS reset variable=cnt_i  
    #pragma HLS reset variable=tmp  
    #pragma HLS reset variable=raw64  
    #pragma HLS reset variable=temp  
    #pragma HLS reset variable=ddr_addr_out
    
  switch(HarrisFSM)
  {
    case WAIT_FOR_META: 
      printf("DEBUG in pProcPath: WAIT_FOR_META\n");
      //if ( (*image_loaded) == true )
      if (!sImageLoaded.empty())
        {
            if (sImageLoaded.read() == true) {
                HarrisFSM = PROCESSING_PACKET;
                //	*processed_word_rx = 0;
                //	*processed_bytes_rx = 0;
                accel_called = false;
                processed_word_proc = 0;
                #ifdef ENABLE_DDR
                ddr_addr_out = 0;
                timeoutCntAbs = 0;
                cnt_i = 0;
                //*image_loaded = false;
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
      printf("DEBUG in pProcPath: HARRIS_RETURN_RESULTS, ddr_addr_out=%u\n", ddr_addr_out);      
      if (accel_called == true) {
	
        printf("DEBUG in pProcPath: Accumulated %u net words (%u B) to complete a single DDR word\n", 
	       KWPERMDW_512, BPERMDW_512);
        if ( img_out_axi_stream.empty() ) {
            tmp = lcl_mem1[ddr_addr_out];
            ddr_addr_out++;
            HarrisFSM = HARRIS_RETURN_RESULTS_ABSORB_DDR_LAT;
            timeoutCntAbs = 0;
        }
      }
    break;
    
    case HARRIS_RETURN_RESULTS_ABSORB_DDR_LAT:
      printf("DEBUG in pProcPath: HARRIS_RETURN_RESULTS_ABSORB_DDR_LAT [%u out of %u]\n", timeoutCntAbs, DDR_LATENCY);        
        if (timeoutCntAbs++ == DDR_LATENCY) {
            HarrisFSM = HARRIS_RETURN_RESULTS_UNPACK;
            cnt_i = 0;
        }
    break;
    
    case HARRIS_RETURN_RESULTS_UNPACK:
      printf("DEBUG in pProcPath: HARRIS_RETURN_RESULTS_UNPACK, cnt_i=%u\n", cnt_i);        
        temp.keep = 0;
        temp.last = 0;
        //for (unsigned int i=0; i<(MEMDW_512/OUTPUT_PTR_WIDTH); i++) {
            #if OUTPUT_PTR_WIDTH == 64
            //raw64 = tmp(i*OUTPUT_PTR_WIDTH, (i+1)*OUTPUT_PTR_WIDTH-1);
            raw64(56,63) = tmp(cnt_i*OUTPUT_PTR_WIDTH+56, cnt_i*OUTPUT_PTR_WIDTH+63);
            raw64(48,55) = tmp(cnt_i*OUTPUT_PTR_WIDTH+48, cnt_i*OUTPUT_PTR_WIDTH+55);
            raw64(40,47) = tmp(cnt_i*OUTPUT_PTR_WIDTH+40, cnt_i*OUTPUT_PTR_WIDTH+47);
            raw64(32,39) = tmp(cnt_i*OUTPUT_PTR_WIDTH+32, cnt_i*OUTPUT_PTR_WIDTH+39);
            raw64(24,31) = tmp(cnt_i*OUTPUT_PTR_WIDTH+24, cnt_i*OUTPUT_PTR_WIDTH+31);
            raw64(16,23) = tmp(cnt_i*OUTPUT_PTR_WIDTH+16, cnt_i*OUTPUT_PTR_WIDTH+23);
            raw64(8 ,15) = tmp(cnt_i*OUTPUT_PTR_WIDTH+8 , cnt_i*OUTPUT_PTR_WIDTH+15);
            raw64(0 ,7 ) = tmp(cnt_i*OUTPUT_PTR_WIDTH   , cnt_i*OUTPUT_PTR_WIDTH+7);
            temp.data = raw64; 
            #endif
            if ( !img_out_axi_stream.full() ) {
                img_out_axi_stream.write(temp);
            }
            if (cnt_i == (MEMDW_512/OUTPUT_PTR_WIDTH) - 1) {
                HarrisFSM = HARRIS_RETURN_RESULTS_FWD;
            }
            cnt_i++;
        //}
      //}
    break;
    
    case HARRIS_RETURN_RESULTS_FWD: 
      printf("DEBUG in pProcPath: HARRIS_RETURN_RESULTS_FWD\n");
      if ( !img_out_axi_stream.empty() && !sRxpToTxp_Data.full() ) {
        temp = img_out_axi_stream.read();
        if (processed_word_proc++ == MIN_TX_LOOPS-1) {
            temp.last = 1;
            HarrisFSM = WAIT_FOR_META;
            //accel_called = false;
        }
        else {
            temp.last = 0;
        }
        //TODO: find why Vitis kernel does not set keep and last by itself
        temp.keep = 255;
        newWord = NetworkWord(temp.data, temp.keep, temp.last); 
        sRxpToTxp_Data.write(newWord);
      }
      else {
        HarrisFSM = HARRIS_RETURN_RESULTS;
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
        //Harris app needs to be reset to process new rank
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
const unsigned int ddr_latency = DDR_LATENCY;

// Mapping LCL_MEM0 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem0 bundle=moMEM_Mp1\
  max_read_burst_length=256  max_write_burst_length=256 offset=direct \
  num_read_outstanding=16 num_write_outstanding=16 latency=ddr_latency

// Mapping LCL_MEM1 interface to moMEM_Mp1 channel
#pragma HLS INTERFACE m_axi depth=ddr_mem_depth port=lcl_mem1 bundle=moMEM_Mp1 \
  max_read_burst_length=256  max_write_burst_length=256 offset=direct \
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
  //static bool image_loaded;
  static stream<bool> sImageLoaded("sImageLoaded");
  static bool skip_read;
  static bool write_chunk_to_ddr_pending;
  //static stream<bool> sWriteChunkToDdrPending("sWriteChunkToDdrPending");
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
  //*po_rx_ports = 0x1; //currently work only with default ports...
  static stream<NodeId>            sDstNode_sig("sDstNode_sig");


  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS stream variable=sRxtoTx_Meta depth=tot_transfers
#pragma HLS reset variable=enqueueFSM
#pragma HLS reset variable=dequeueFSM
#pragma HLS reset variable=HarrisFSM
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

#ifndef ENABLE_DDR
#pragma HLS stream variable=img_in_axi_stream depth=img_in_axi_stream_depth
#pragma HLS stream variable=img_out_axi_stream depth=img_out_axi_stream_depth
#else
#pragma HLS reset variable=fsmStateDDR
#endif




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
               &processed_word_tx,
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
  
#ifdef ENABLE_DDR
  
 pPortAndDestionation(pi_rank, pi_size, sDstNode_sig, po_rx_ports);
  
  
 pRXPathDDR(
    siSHL_This_Data,
    siNrc_meta,
    sRxtoTx_Meta,
    //---- P0 Write Path (S2MM) -----------
    soMemWrCmdP0,
    siMemWrStsP0,
    soMemWriteP0,
    // ---- P1 Memory mapped --------------
    lcl_mem0,
    meta_tmp,
    &processed_word_rx,
    &processed_word_tx,
    &processed_bytes_rx,
    //&image_loaded
    sImageLoaded  
    );

 #else // !ENABLE_DDR
  
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
    &processed_word_tx,
    &processed_bytes_rx,
    //&image_loaded,
    sImageLoaded,
    &skip_read,
    &write_chunk_to_ddr_pending,
    //sWriteChunkToDdrPending,
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
        //&image_loaded
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
        pi_size);
#endif // USE_HLSLIB_DATAFLOW
}


/*! \} */
