/*****************************************************************************
 * @file       memtest.cpp
 * @brief      The Role for a Memtest Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID
 * @date       May 2020
 * @updates    DCO
 * @date       September 2021
 *----------------------------------------------------------------------------
 *
 * @details      This application implements a UDP/TCP-oriented Vitis function.
 *
 * @deprecated   For the time being, we continue designing with the DEPRECATED
 *               directives because the new PRAGMAs do not work for us.
 * 
 *----------------------------------------------------------------------------
 * 
 * @ingroup MemtestHLS
 * @addtogroup MemtestHLS
 * \{
 *****************************************************************************/

#include "../include/memtest.hpp"
#include "../include/mem_pattern.hpp"
#include "../../../../../HOST/custom/memtest/languages/cplusplus/include/config.h"

#ifdef USE_HLSLIB_DATAFLOW
#include "../../../../../hlslib/include/hlslib/xilinx/Stream.h"
#include "../../../../../hlslib/include/hlslib/xilinx/Simulation.h"
#endif

#ifdef USE_HLSLIB_STREAM
using hlslib::Stream;
#endif
using hls::stream;

#define Data_t_in  ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0>
#define Data_t_out ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0>

PacketFsmType enqueueFSM = WAIT_FOR_META;
PacketFsmType dequeueFSM = WAIT_FOR_STREAM_PAIR;

typedef char word_t[8];

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
    ap_uint<32>                 *po_rx_ports
    )
{
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS INLINE off
  //-- STATIC VARIABLES (with RESET) ------------------------------------------
  static PortFsmType port_fsm = FSM_WRITE_NEW_DATA;
#pragma HLS reset variable=port_fsm
#pragma HLS reset variable=MemtestFSM



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
 * @brief Receive Path - From SHELL to THIS.
 *
 * @param[in]  siSHL_This_Data
 * @param[in]  siNrc_meta
 * @param[out] sRxtoTx_Meta
 * @param[out] sRxpToProcp_Data
 * @param[out] start_stop
 * @param[out] meta_tmp
 * @param[out] processed_word
 *
 * @return Nothing.
 ******************************************************************************/
void pRXPath(
  stream<NetworkWord>                              &siSHL_This_Data,
        stream<NetworkMetaStream>                        &siNrc_meta,
  stream<NetworkMetaStream>                        &sRxtoTx_Meta,
  stream<NetworkWord>                              &sRxpToProcp_Data,
  NetworkMetaStream                                meta_tmp,
  bool  *                                           start_stop,
  unsigned int                                     *processed_word_rx,
  unsigned int                                     *processed_bytes_rx
      )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    //#pragma HLS DATAFLOW interval=1
     #pragma  HLS INLINE 
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord    netWord;
    // word_t text;
    //UppercaseCmd fsmCmd;
    static bool start_stop_local = false;
#pragma HLS reset variable=start_stop_local

    *start_stop = start_stop_local;
  switch(enqueueFSM)
  {
    case WAIT_FOR_META: 
      printf("DEBUG in pRXPath: enqueueFSM - WAIT_FOR_META, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
       *processed_word_rx, *processed_bytes_rx);
      if ( !siNrc_meta.empty() && !sRxtoTx_Meta.full() )
      {
        meta_tmp = siNrc_meta.read();//not sure if I have to continue to test or not, hence sending the meta or not is different
        meta_tmp.tlast = 1; //just to be sure...
        sRxtoTx_Meta.write(meta_tmp);
        enqueueFSM = PROCESSING_PACKET;
      }
     // *start_stop = start_stop_local;
      break;

    case PROCESSING_PACKET:
      printf("DEBUG in pRXPath: enqueueFSM - PROCESSING_PACKET, *processed_word_rx=%u, *processed_bytes_rx=%u\n",
       *processed_word_rx, *processed_bytes_rx);
      //*start_stop = start_stop_local;
      
      if ( !siSHL_This_Data.empty() && !sRxpToProcp_Data.full() )
      {
        //-- Read incoming data chunk
        netWord = siSHL_This_Data.read();

        switch(netWord.tdata.range(1,0))//the command is in the first two bits
        {
          case(TEST_START_CMD):
            start_stop_local=true;
            *start_stop=true;
            netWord.tdata.range(1,0)=TEST_START_CMD;//28506595412;//"B_ACK";
            netWord.tdata.range(NETWORK_WORD_BIT_WIDTH-1,NETWORK_WORD_BIT_WIDTH-32);//32 bits for the number of addresses to test 
            sRxpToProcp_Data.write(netWord);
            break;
          case(TEST_STOP_CMD):
            start_stop_local=false;
            *start_stop=false;
            netWord.tdata=TEST_STOP_CMD;//358080398155;//"S_ACK" string
            netWord.tlast = 1;
            sRxpToProcp_Data.write(netWord);
            break;
          default:
            if (start_stop_local)
            {
              //some data manipulation here
              // everything is running and should no sending anything back
            } else {
              netWord.tdata.range(1,0)=TEST_INVLD_CMD;
              sRxpToProcp_Data.write(netWord);
            }
            break;

        } 
        //no need to forwarding every packet to the processing, hence commenting out
        //sRxpToProcp_Data.write(netWord);
        if(netWord.tlast == 1)
        {
          enqueueFSM = WAIT_FOR_META;
        }
      }
      break;
  }

 
}


/*****************************************************************************
 * @brief THIS processing the data once recieved a start command
 *
 * @param[in]  sRxpToProcp_Data
 * @param[out] sProcpToTxp_Data
 * @param[in]  start_stop
 *
 * @return Nothing.
 ******************************************************************************/
 void pTHISProcessingData(
  stream<NetworkWord>                              &sRxpToProcp_Data,
  stream<NetworkWord>                              &sProcpToTxp_Data,
  bool *                                             start_stop
  ){

    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma  HLS INLINE off
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord    netWord;
    NetworkWord    outNetWord;
    word_t text;
    static local_mem_word_t local_under_test_memory [LOCAL_MEM_ADDR_SIZE];
    static local_mem_addr_t max_address_under_test; // byte addressable;
    static ap_uint<32> local_mem_addr_non_byteaddressable;
    static local_mem_addr_t curr_address_under_test;
    static local_mem_addr_t first_faulty_address;
    static ap_uint<32> faulty_addresses_cntr;
    static ProcessingFsmType processingFSM  = FSM_PROCESSING_STOP;

    static ap_uint<30> testCounter;

#pragma HLS reset variable=testCounter
#pragma HLS reset variable=local_under_test_memory
#pragma HLS reset variable=max_address_under_test
#pragma HLS reset variable=local_mem_addr_non_byteaddressable
#pragma HLS reset variable=curr_address_under_test
#pragma HLS reset variable=first_faulty_address
#pragma HLS reset variable=faulty_addresses_cntr
#pragma HLS reset variable=address_under_test
#pragma HLS reset variable=processingFSM


//assuming that whnever I send a start I must complete the run and then restart unless a stop
// or stopping once done with the run
    switch(processingFSM)
    {
      case FSM_PROCESSING_STOP:
      /////////////////////////////////////////////////////////////////////////////////////////////
      //TODO adding logic for saving the meta and sending continuously at the end of each test????
      /////////////////////////////////////////////////////////////////////////////////////////////
      //resetting once per test suite
      max_address_under_test = 0; 

      printf("DEBUG proc FSM, I am in the stop state\n");
      if ( !sRxpToProcp_Data.empty() ) //&& !sProcpToTxp_Data.full() )
      {
        //-- Read incoming data chunk
        netWord = sRxpToProcp_Data.read();
        //sProcpToTxp_Data.write(netWord);
      }
      if ( *start_stop )
      {
        max_address_under_test = netWord.tdata.range(NETWORK_WORD_BIT_WIDTH-1,NETWORK_WORD_BIT_WIDTH-32);
        processingFSM = FSM_PROCESSING_START;

      } else {
        testCounter = 0;
      }
      break;

    case FSM_PROCESSING_START:
      //resetting each execution to the 0 address
      curr_address_under_test = 0; 
      first_faulty_address = 0; 
      faulty_addresses_cntr = 0;
      local_mem_addr_non_byteaddressable = 0;

      if ( *start_stop && testCounter != 0)
      {
        testCounter += 1;
        processingFSM = FSM_PROCESSING_WRITE;

      } else {

        testCounter = 0;
        processingFSM = FSM_PROCESSING_STOP;

      }
      break;


    case FSM_PROCESSING_WRITE:
    //if not written all the needed memory cells write
      if (curr_address_under_test != max_address_under_test)
      {
        memcpy(local_under_test_memory+local_mem_addr_non_byteaddressable, lorem_ipsum_pattern+curr_address_under_test, LOCAL_MEM_WORD_BYTE_SIZE);
        local_mem_addr_non_byteaddressable += 1;
        curr_address_under_test += LOCAL_MEM_ADDR_OFFSET;

      } else{
        
        local_mem_addr_non_byteaddressable = 0;
        curr_address_under_test = 0;
        processingFSM = FSM_PROCESSING_READ;

      }
      break;


    case FSM_PROCESSING_READ:

    //if not read all the needed memory cells read
      if (curr_address_under_test != max_address_under_test)
      {
        char readingString [LOCAL_MEM_WORD_BYTE_SIZE];
        memcpy(readingString,local_under_test_memory+local_mem_addr_non_byteaddressable,LOCAL_MEM_WORD_BYTE_SIZE);
        for (int i = 0; i < curr_address_under_test+LOCAL_MEM_ADDR_OFFSET; ++i)
        {
          if (readingString[i] != lorem_ipsum_pattern[i+curr_address_under_test]) // fault check
          {
            if (faulty_addresses_cntr == 0) //first fault
            {
              first_faulty_address = i+curr_address_under_test; //save the fault address
            }
            faulty_addresses_cntr += 1; //increment the fault counter
          } 
        }
        curr_address_under_test += LOCAL_MEM_ADDR_OFFSET; //next test
        local_mem_addr_non_byteaddressable += 1;

      } else{ //done with the reads

        if (!sProcpToTxp_Data.full() )
        {
          //wrtie first part so the result so how many addresses had problems
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 0;
          outNetWord.tdata = faulty_addresses_cntr;
          processingFSM = FSM_PROCESSING_OUTPUT;
        }

      }
      break;

    case FSM_PROCESSING_OUTPUT:
      if (!sProcpToTxp_Data.full() )
      {
          //wrtie second part of the result, so which was the first address with problems
          outNetWord.tkeep = 0xFF;
          outNetWord.tlast = 1;
          outNetWord.tdata = first_faulty_address;
          processingFSM = FSM_PROCESSING_START;
      }
      break;

  }
 };



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
  stream<NodeId>          &sDstNode_sig,
        unsigned int                *processed_word_tx, 
        ap_uint<32>                 *pi_rank
      )
{
    //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
    //#pragma HLS DATAFLOW interval=1
    #pragma  HLS INLINE
    //-- LOCAL VARIABLES ------------------------------------------------------
    NetworkWord      netWordTx;
    NetworkMeta  meta_in = NetworkMeta();
    static NodeId dst_rank;
  
  switch(dequeueFSM)
  {
    case WAIT_FOR_META:
      if(!sDstNode_sig.empty())
      {
        dst_rank = sDstNode_sig.read();
        dequeueFSM = WAIT_FOR_STREAM_PAIR;
        //Triangle app needs to be reset to process new rank
      }
      break;
    case WAIT_FOR_STREAM_PAIR:
      printf("DEBUG in pTXPath: dequeueFSM=%d - WAIT_FOR_STREAM_PAIR, *processed_word_tx=%u\n", 
       dequeueFSM, *processed_word_tx);
      //-- Forward incoming chunk to SHELL
      *processed_word_tx = 0;
      
      /*
      printf("!sRxpToProcp_Data.empty()=%d\n", !sRxpToProcp_Data.empty());
      printf("!sRxtoTx_Meta.empty()=%d\n", !sRxtoTx_Meta.empty());
      printf("!soTHIS_Shl_Data.full()=%d\n", !soTHIS_Shl_Data.full());
      printf("!soNrc_meta.full()=%d\n", !soNrc_meta.full());
      */
      
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

        meta_out_stream.tdata.dst_rank = dst_rank;
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
      dequeueFSM = WAIT_FOR_STREAM_PAIR;
  }
  
        soTHIS_Shl_Data.write(netWordTx);
      }
      break;
  }
}


/*****************************************************************************
 * @brief   Main process of the Memtest Application 
 * directives.
 * @deprecated  This functions is using deprecated AXI stream interface 
 * @return Nothing.
 *****************************************************************************/
void memtest(

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
  // static stream<NetworkWord>       sRxpToProcp_Data("sRxpToProcp_Data"); // FIXME: works even with no static
  NetworkMetaStream  meta_tmp = NetworkMetaStream();
  static stream<NetworkMetaStream> sRxtoTx_Meta("sRxtoTx_Meta");
  static stream<NetworkWord>       sProcpToTxp_Data("sProcpToTxp_Data"); // FIXME: works even with no static
  static stream<NetworkWord>       sRxpToProcp_Data("sRxpToProcp_Data"); // FIXME: works even with no static



  static unsigned int processed_word_rx;
  static unsigned int processed_bytes_rx;
  static unsigned int processed_word_tx;
  //*po_rx_ports = 0x1; //currently work only with default ports...
  static stream<NodeId>            sDstNode_sig   ("sDstNode_sig");
  bool                              start_stop;
  //-- DIRECTIVES FOR THIS PROCESS ------------------------------------------
#pragma HLS DATAFLOW 
#pragma HLS reset variable=enqueueFSM
#pragma HLS reset variable=dequeueFSM
#pragma HLS reset variable=processed_word_rx
#pragma HLS reset variable=processed_word_tx

  

#ifdef USE_HLSLIB_DATAFLOW
  /*! @copybrief uppercase()
   *  Uppercase is eanbled with hlslib support
   */
  /*! @copydoc uppercase()
   * Use this snippet to early check for C++ errors related to dataflow and bounded streams (empty 
   * and full) during simulation. It can also be both synthesized and used in co-simulation.
   * Practically we use hlslib when we want to run simulation as close as possible to the HW, by 
   * executing all functions of dataflow in thread-safe parallel executions, i.e the function 
   * HLSLIB_DATAFLOW_FINALIZE() acts as a barrier for the threads spawned to serve every function 
   * called in HLSLIB_DATAFLOW_FUNCTION(func, args...).
   */
   /*! @copydetails uppercase()
   * hlslib is a collection of C++ headers, CMake files, and examples, aimed at improving the 
   * quality of life of HLS developers. More info at: https://github.com/definelicht/hlslib
   */
  // Dataflow functions running in parallel
  HLSLIB_DATAFLOW_INIT();
  
  HLSLIB_DATAFLOW_FUNCTION(pRXPath, 
         siSHL_This_Data,
         siNrc_meta,
         sRxtoTx_Meta,
         meta_tmp,
         sRxpToProcp_Data,
         &processed_word_rx,
         &processed_bytes_rx);

  HLSLIB_DATAFLOW_FUNCTION(pTXPath,
         soTHIS_Shl_Data,
         soNrc_meta,
         sRxpToProcp_Data,
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
  po_rx_ports);

 pRXPath(
  siSHL_This_Data,
  siNrc_meta,
  sRxtoTx_Meta,
  sRxpToProcp_Data,
  meta_tmp,
  &start_stop,
  &processed_word_rx,
  &processed_bytes_rx);

 pTHISProcessingData(
  sRxpToProcp_Data,
  sProcpToTxp_Data,
  &start_stop);

  
  pTXPath(
  soTHIS_Shl_Data,
  soNrc_meta,
  sProcpToTxp_Data,
  sRxtoTx_Meta,
  sDstNode_sig,
  &processed_word_tx,
  pi_rank);
#endif // USE_HLSLIB_DATAFLOW
}


/*! \} */
