  
#ifndef _ROLE_MEMTEST_PATTERN_H_
#define _ROLE_MEMTEST_PATTERN_H_

#include "../include/memtest_library.hpp"

#define FAULT_INJECTION // macro for fault injection insertion

#define FSM_PROCESSING_WAIT_FOR_META 0
#define FSM_PROCESSING_PCKT_PROC 1
#define FSM_PROCESSING_STOP 2
#define FSM_PROCESSING_START 3
#define FSM_PROCESSING_DATAFLOW_WRITE 4
#define FSM_PROCESSING_DATAFLOW_READ 5
#define FSM_PROCESSING_OUTPUT 6
#define FSM_PROCESSING_OUTPUT_2 7
#define FSM_PROCESSING_OUTPUT_3 8
#define FSM_PROCESSING_OUTPUT_4 9
#define FSM_PROCESSING_OUTPUT_5 10
#define FSM_PROCESSING_CONTINUOUS_RUN 11

#define ProcessingFsmType uint8_t


#define LOCAL_MEM_WORD_SIZE 512
#define LOCAL_MEM_ADDR_SIZE 20
#define MEMTEST_ADDRESS_BITWIDTH 40
#define MEMTEST_ITERATION_BITWIDTH 16


typedef ap_uint<LOCAL_MEM_WORD_SIZE>  local_mem_word_t;
typedef ap_uint<MEMTEST_ADDRESS_BITWIDTH>  local_mem_addr_t; 
#define LOCAL_MEM_ADDR_SIZE_NON_BYTE_ADDRESSABLE 16 // TODO: to parametrize better
typedef ap_uint<LOCAL_MEM_ADDR_SIZE_NON_BYTE_ADDRESSABLE>  local_mem_addr_non_byteaddressable_t; 
#define LOCAL_MEM_ADDR_OFFSET (LOCAL_MEM_WORD_SIZE/8) //byte addres offset
#define LOCAL_MEM_WORD_BYTE_SIZE (LOCAL_MEM_WORD_SIZE/8) //byte size of a local mem word

#define MEMTEST_ADDRESS_HIGH_BIT NETWORK_WORD_BIT_WIDTH-1 // 63
#define MEMTEST_ADDRESS_LOW_BIT NETWORK_WORD_BIT_WIDTH-MEMTEST_ADDRESS_BITWIDTH //64-40 = 24

#define MEMTEST_ITERATIONS_HIGH_BIT MEMTEST_ADDRESS_LOW_BIT-1 // 23
#define MEMTEST_ITERATIONS_LOW_BIT  MEMTEST_ITERATIONS_HIGH_BIT+1-MEMTEST_ITERATION_BITWIDTH // 

#define MAX_ITERATION_COUNT 10
const unsigned int max_proc_fifo_depth = MAX_ITERATION_COUNT;

/********************************************
Functions for generating random sequences
********************************************/
template<typename Tin, typename Tout>
Tout genNextFibonacciNumber(Tin curr, Tin prev){
#pragma HLS INLINE
  return static_cast<Tout>(curr + prev);
}

template<typename ADDR_T, unsigned int sequenceDim, typename BIGWORD_T, typename SMALLWORD_T, unsigned int smallWordDim>
void genFibonacciNumbers(ADDR_T curr, BIGWORD_T * outBigWord){
//#pragma HLS INLINE off

// REQUIREMENT: must be a multiple
  //assert( (BIGWORD_T%SMALLWORD_T)==0);

  SMALLWORD_T currentFibonacciNumber = static_cast<SMALLWORD_T>(curr);
  SMALLWORD_T prevFibonacciNumber = currentFibonacciNumber + 1;
  SMALLWORD_T nextFibonacciNumber = genNextFibonacciNumber<ADDR_T,SMALLWORD_T>(currentFibonacciNumber,prevFibonacciNumber);

  gen_sequence_loop: for (unsigned int i = 0; i < sequenceDim; i++)
  {
#pragma HLS PIPELINE
    (*outBigWord).range(smallWordDim*(i+1)-1,smallWordDim*i)=nextFibonacciNumber;
    prevFibonacciNumber=currentFibonacciNumber;
    currentFibonacciNumber=nextFibonacciNumber;
    nextFibonacciNumber=genNextFibonacciNumber<ADDR_T,SMALLWORD_T>(currentFibonacciNumber,prevFibonacciNumber);
  }
  

}


template<typename ADDR_T, unsigned int sequenceDim, typename BIGWORD_T, typename SMALLWORD_T, unsigned int smallWordDim>
void genXoredSequentialNumbers(ADDR_T curr, BIGWORD_T * outBigWord){
#pragma HLS INLINE off

// REQUIREMENT: must be a multiple
  //assert( (BIGWORD_T%SMALLWORD_T)==0);

  SMALLWORD_T currentNumber = static_cast<SMALLWORD_T>(curr);
  SMALLWORD_T nextNumber = (currentNumber+1) xor 1;
  SMALLWORD_T prevNumber = currentNumber;

  gen_sequence_loop: for (unsigned int i = 0; i < sequenceDim; i++)
  {
#pragma HLS PIPELINE
    (*outBigWord).range(smallWordDim*(i+1)-1,smallWordDim*i)=nextNumber;
    prevNumber = currentNumber;
    currentNumber = nextNumber;
    nextNumber = (nextNumber + 1 ) xor i;
  }
}


template<typename ADDR_T, unsigned int sequenceDim, typename BIGWORD_T, typename SMALLWORD_T, unsigned int smallWordDim>
void genXoredSequentialNumbersSecondVersion(ADDR_T curr, BIGWORD_T * outBigWord){
#pragma HLS INLINE 
// REQUIREMENT: must be a multiple
  //assert( (BIGWORD_T%SMALLWORD_T)==0);
  SMALLWORD_T currentNumber = static_cast<SMALLWORD_T>(curr);
  SMALLWORD_T nextNumber = (currentNumber+1) xor 1;
  SMALLWORD_T prevNumber = currentNumber;

  gen_sequence_loop: for (unsigned int i = 0; i < sequenceDim; i++)
  {
#pragma HLS UNROLL
    (*outBigWord).range(smallWordDim*(i+1)-1,smallWordDim*i)=nextNumber;
    prevNumber = currentNumber;
    currentNumber = nextNumber;
    nextNumber = (nextNumber + 1 ) xor i;
  }
}


template<typename ADDR_T, typename BIGWORD_T>
void genSequentialNumbers(ADDR_T curr, BIGWORD_T * outBigWord){
#pragma HLS INLINE 
  *outBigWord = curr+1;
}


template<const unsigned int max_iterations=4000000, const unsigned int buff_dim = 16>
void pWRGenerateData2StreamWrite(
  hls::stream<local_mem_word_t>& generatedData,
  ap_uint<32> * testCounter,
  local_mem_addr_t max_addr_ut)
{
#pragma HLS INLINE off
    static local_mem_word_t tmp_out [buff_dim];
#pragma HLS array_partition variable=tmp_out cyclic factor=2 dim=1

    local_mem_addr_non_byteaddressable_t maddr_non_byte=0;
    local_mem_addr_t curr_address_ut;


  generate_loop:
  for (curr_address_ut = 0, maddr_non_byte=0; curr_address_ut < max_addr_ut; curr_address_ut+=LOCAL_MEM_ADDR_OFFSET)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    
    genXoredSequentialNumbersSecondVersion
    <local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32, local_mem_word_t,
     ap_uint<32>,32>(maddr_non_byte, 
      tmp_out+(maddr_non_byte%buff_dim));

    local_mem_word_t tmp_out_scalar;
    #ifdef FAULT_INJECTION
         //TODO:  place for control fault injection with a function?
         if(*testCounter >= 2 && maddr_non_byte > 0){
            // tmp_out = local_under_test_memory[local_mem_addr_non_byteaddressable] & static_cast<local_mem_word_t>(0);
            tmp_out_scalar = tmp_out[maddr_non_byte%buff_dim] & static_cast<local_mem_word_t>(0);
          }else{
             tmp_out_scalar = tmp_out[maddr_non_byte%buff_dim];
          }
    #else // FAULT_INJECTION
      tmp_out_scalar = tmp_out[maddr_non_byte%buff_dim];
    #endif // FAULT_INJECTION

    generatedData.write(tmp_out_scalar);
    maddr_non_byte++;
  }
  

}


template <typename Tcntr, const unsigned int max_iterations=4000000,
const unsigned int buff_dim=64*2>
void pWRReadStream2WriteMainMemory(
  hls::stream<Tcntr>& cmd,
hls::stream<local_mem_word_t>& generatedData,
membus_t * lcl_mem,
local_mem_addr_t max_addr_ut,
unsigned int burst_size)
{
#pragma HLS INLINE off
    local_mem_addr_t curr_address_ut;
    local_mem_addr_t curr_writing_addr;
    static local_mem_word_t tmp_out[buff_dim];
    static unsigned int end_distance=0;
    static bool last_iteration = false;
#pragma HLS array_partition variable=tmp_out block factor=2 dim=1

    cmd.write(0);
  int i, written_i;
  read_and_write:
  for (curr_address_ut = 0, i=0, curr_writing_addr=0, written_i=0; curr_address_ut < max_addr_ut; curr_address_ut+=LOCAL_MEM_ADDR_OFFSET)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    //#ifndef __SYNTHESIS__
    //checking_loop: for (int i = 0; i < LOCAL_MEM_ADDR_OFFSET; i++)
    //       {
    //        std::cout << tmp_out[i].range((i+1)*8-1,i*8) << " ";
    //       }
    //       std::cout << std::endl;
    // #endif
   unsigned int idx = i;
    tmp_out[idx] = generatedData.read();
    //if stored enough data to begin the bursting OR this is last iteration
    end_distance = max_addr_ut-curr_address_ut;
    last_iteration = LOCAL_MEM_ADDR_OFFSET>=end_distance;
    
    //std::cout << "test addr " << idx << " current address " << curr_address_ut <<std::endl;
    if (idx-written_i>=burst_size-1 || last_iteration)
    {
    //std::cout << "Burst filled or last iteration, end_distance= " << end_distance << std::endl;
      if (last_iteration)
      {
        // std::cout << "LAST transferring " << idx%burst_size+1 << " words at " << curr_writing_addr << " address, from  " << written_i << std::endl;
        memcpy(lcl_mem+curr_writing_addr, tmp_out+written_i, sizeof(local_mem_word_t)*(idx%burst_size+1));
        curr_writing_addr+=(idx%burst_size+1)*LOCAL_MEM_ADDR_OFFSET;
        written_i=(written_i+idx%burst_size+1)%buff_dim;
      }else{
        // std::cout << "BURST transferring " << burst_size << " words at " << curr_writing_addr << " address, from  " << written_i << std::endl;
        memcpy(lcl_mem+curr_writing_addr, tmp_out+written_i, sizeof(local_mem_word_t)*burst_size);
        curr_writing_addr+=burst_size*LOCAL_MEM_ADDR_OFFSET;
        written_i= (written_i+burst_size)%buff_dim;
      }
    }
    if(i==buff_dim-1){
      i=0;
    }else{
      i++;
    }
   // std::cout << std::endl;
   // memcpy(lcl_mem+curr_address_ut, tmp_out+curr_address_ut%buff_dim, sizeof(local_mem_word_t));
  }
    cmd.write(0);
    end_distance=0;
    last_iteration = false;
}


template <typename Tcntr, const unsigned int max_iterations=4000000,
const unsigned int buff_dim=64*2>
void pRDMainMemoryRead2StreamData(
  hls::stream<Tcntr>& cmd,
  hls::stream<local_mem_word_t>& readData,
  membus_t * lcl_mem,
  local_mem_addr_t max_addr_ut,
unsigned int burst_size)
{
#pragma HLS INLINE off

    local_mem_addr_t curr_address_ut;
    static local_mem_addr_t curr_reading_addr;
    static local_mem_word_t tmp_out[buff_dim];
#pragma HLS array_partition variable=tmp_out block factor=2 dim=1

  cmd.write(0);
  int i, reading_i;
  read_data_from_main_mem:
  for (i = 0, curr_address_ut = 0, curr_reading_addr=0, reading_i=0; curr_address_ut < max_addr_ut; curr_address_ut+=LOCAL_MEM_ADDR_OFFSET, i++)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations

    memcpy(tmp_out+reading_i, lcl_mem+curr_address_ut, sizeof(local_mem_word_t));
    if(reading_i > curr_reading_addr+1){
      readData.write(tmp_out[curr_reading_addr]);
      curr_reading_addr++;
    }
    //std::cout << "read the " << reading_i << " memory word, outputreading at  " << curr_reading_addr << " i at " << i << std::endl;
    if(reading_i==buff_dim-1){
      reading_i=0;
    }else{
      reading_i++;
    }

  }
  cmd.write(0);
  reading_i=i%buff_dim;
  sent_out_remaining_buff: 
  for (int j = 0; j < buff_dim; j++)
  {
#pragma HLS PIPELINE II=1
    if(j==curr_reading_addr && i != curr_reading_addr){
      //std::cout << " writing a memory word" << std::endl;
      readData.write(tmp_out[curr_reading_addr]);
      curr_reading_addr++;
      tmp_out[j]=0;
    }else{
      tmp_out[j]=0;
    }
  }
  curr_reading_addr=0;

}

template<const unsigned int max_iterations=4000000>
void pRDReadDataStreamAndProduceGold(
  hls::stream<local_mem_word_t>& readData,
  local_mem_addr_t max_addr_ut,
  hls::stream<local_mem_word_t>& outReadData,
  hls::stream<local_mem_word_t>& outGoldData)
{

    local_mem_addr_t curr_address_ut;
    local_mem_word_t testingVector;
    local_mem_word_t goldenVector;
    static local_mem_addr_non_byteaddressable_t local_mem_addr_non_byteaddressable=0;

  generate_loop:
  for (curr_address_ut = 0; curr_address_ut < max_addr_ut; curr_address_ut+=LOCAL_MEM_ADDR_OFFSET)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    testingVector = readData.read(); 

    genXoredSequentialNumbersSecondVersion<local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32,
     local_mem_word_t, ap_uint<32>,32>(local_mem_addr_non_byteaddressable, &goldenVector);

    outReadData.write(testingVector);
    outGoldData.write(goldenVector);
    local_mem_addr_non_byteaddressable++;
  }
  local_mem_addr_non_byteaddressable=0;
  
}

template<const unsigned int max_iterations=4000000, const unsigned int unrolling_factor= LOCAL_MEM_ADDR_OFFSET, //inability of using define in pragmas solved in 2021.1
 const unsigned int buff_dim=16>
void pRDCompareDataStreamsCount(
  local_mem_addr_t max_addr_ut,
  hls::stream<local_mem_word_t>& sInReadData,
  hls::stream<local_mem_word_t>& sInGoldData,
  ap_uint<32> * faulty_addresses_cntr,
  local_mem_addr_t * first_faulty_address)
{
//#pragma HLS INLINE off
    const unsigned int dble_wrd_dim =  LOCAL_MEM_ADDR_OFFSET * 2;
    const unsigned int support_dim  =  LOCAL_MEM_ADDR_OFFSET  * 2;


    local_mem_addr_t curr_address_ut;
    static local_mem_word_t testingVector[buff_dim];
    static local_mem_word_t goldenVector[buff_dim];
    local_mem_addr_non_byteaddressable_t maddr_non_byte=0;


    static ap_uint<8> testingVector_bytes [support_dim];
    static ap_uint<8> goldenVector_bytes [support_dim];
    static bool cmp_ok [support_dim];
    static ap_uint<32> faulty_addresses_cntr_support_array [support_dim];

#pragma HLS array_partition variable=faulty_addresses_cntr_support_array cyclic factor=2 dim=1
#pragma HLS array_partition variable=testingVector_bytes cyclic factor=2 dim=1
#pragma HLS array_partition variable=goldenVector_bytes cyclic factor=2 dim=1
#pragma HLS array_partition variable=cmp_ok cyclic factor=2 dim=1

    static bool first_fault_found  = false;
    static ap_uint<32> faulty_addresses_cntr_local;

    ap_uint<1> k;
      reading_loop:
  for (curr_address_ut = 0, k=0, faulty_addresses_cntr_local=0; curr_address_ut < max_addr_ut; curr_address_ut+=LOCAL_MEM_ADDR_OFFSET, k++)
  {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
  testingVector[maddr_non_byte%buff_dim] = sInReadData.read(); 
  goldenVector[maddr_non_byte%buff_dim] = sInGoldData.read(); 

      //ap_uint<64> tmpOut = 0; 

   //#pragma HLS dependence variable=faulty_addresses_cntr_support_array inter RAW false
      golden_comparison: for (int i = 0; i < LOCAL_MEM_ADDR_OFFSET; i++)
   {
//#pragma HLS UNROLL factor=unrolling_factor skip_exit_check
        //int idx = (i + curr_address_ut )% support_dim;
        int idx = (i + k*LOCAL_MEM_ADDR_OFFSET);
        testingVector_bytes[idx]=testingVector[maddr_non_byte%buff_dim].range((i+1)*8-1,i*8);
        goldenVector_bytes[idx]=goldenVector[maddr_non_byte%buff_dim].range((i+1)*8-1,i*8);
        //std::cout << " tst=" << testingVector_bytes[idx] << " vs gld=" << goldenVector_bytes[idx] ;

      cmp_ok[idx] = testingVector_bytes[idx] == goldenVector_bytes[idx];
     // tmpOut[i] = !cmp_ok[idx];
      if(!cmp_ok[idx] ){
        faulty_addresses_cntr_support_array[i+k]++;
        if (!first_fault_found)
        {
          *first_faulty_address=i+curr_address_ut;
          first_fault_found = true;
        }else{
          first_fault_found = first_fault_found;
        }
      }
   }
   //std::cout << std::endl;
   //sInCmpRes.write(tmpOut);
    maddr_non_byte++;
  }

  for (int i = 0; i < support_dim; i++)
  {
#pragma HLS PIPELINE
    faulty_addresses_cntr_local += faulty_addresses_cntr_support_array[i];
    faulty_addresses_cntr_support_array[i]=0;
  }
  
  first_fault_found=false;
*faulty_addresses_cntr=faulty_addresses_cntr_local;

}


void pWriteDataflowMemTest(
  membus_t * lcl_mem0,
  local_mem_addr_t max_address_under_test,
  ap_uint<64> * writing_cntr,
  ap_uint<32> * testCounter)
{
  #pragma HLS INLINE off
     static hls::stream<ap_uint<64>> sWritePrfCntr_cmd("sWritePrfCntr_cmd"); 
     #pragma HLS STREAM variable=sWritePrfCntr_cmd depth=64 dim=1
     static hls::stream<local_mem_word_t> generatedWriteData("generatedWriteData"); 
     #pragma HLS STREAM variable=generatedWriteData depth=64 dim=1
    #pragma HLS DATAFLOW
          //Step 1: Generate the data
          pWRGenerateData2StreamWrite<4000000>(generatedWriteData,testCounter,max_address_under_test);
          //Step 2: write 
          pWRReadStream2WriteMainMemory<ap_uint<64>,4000000>(sWritePrfCntr_cmd, generatedWriteData, lcl_mem0, max_address_under_test, 1);
          //Step 2.b: count 
          perfCounterProc2Mem<ap_uint<64>,ap_uint<64>,64>(sWritePrfCntr_cmd, writing_cntr, 0, 256,  16);
      

}


void pReadDataflowMemTest(
  membus_t * lcl_mem1,
  local_mem_addr_t max_address_under_test,
  ap_uint<64> * reading_cntr,
  ap_uint<32> * faulty_addresses_cntr,
  local_mem_addr_t * first_faulty_address)
  {
  #pragma HLS INLINE off

 static hls::stream<ap_uint<64>> sReadPrfCntr_cmd("sReadPrfCntr_cmd"); 
 #pragma HLS STREAM variable=sReadPrfCntr_cmd depth=2 dim=1
 static hls::stream<local_mem_word_t> generatedReadData("generatedReadData"); 
 #pragma HLS STREAM variable=generatedReadData depth=64 dim=1
  static hls::stream<local_mem_word_t> sReadData("sReadData"); 
 #pragma HLS STREAM variable=sReadData depth=64 dim=1
  static hls::stream<local_mem_word_t> sGoldData("sGoldData"); 
 #pragma HLS STREAM variable=sGoldData depth=64 dim=1

  static hls::stream<ap_uint<64>> sComparisonData("sComparisonData"); 
 #pragma HLS STREAM variable=sComparisonData depth=64 dim=1
 //  static hls::stream<local_mem_addr_t> sFaultyAddresses("sFaultyAddresses"); 
 // #pragma HLS STREAM variable=sFaultyAddresses depth=64 dim=1

#pragma HLS DATAFLOW
      //Step 1: Generate the data
      pRDMainMemoryRead2StreamData<ap_uint<64>,4000000>( sReadPrfCntr_cmd, generatedReadData, lcl_mem1, max_address_under_test,1);
      //Step 2: write 
      pRDReadDataStreamAndProduceGold<4000000>(generatedReadData, max_address_under_test, sReadData, sGoldData);
      //Step 2.b: count 
      perfCounterProc2Mem<ap_uint<64>,ap_uint<64>,64>(sReadPrfCntr_cmd, reading_cntr, 0, 256,  16);
      //Step 3: compare
      pRDCompareDataStreamsCount<4000000>(max_address_under_test,sReadData, sGoldData,faulty_addresses_cntr, first_faulty_address);
}



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
 *
 * @param[in]  sRxpToProcp_Data
 * @param[out] sProcpToTxp_Data
 * @param[in]  start_stop
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
#pragma HLS reset variable=first_faulty_address
    static ap_uint<32> faulty_addresses_cntr;
#pragma HLS reset variable=faulty_addresses_cntr
    static local_mem_addr_t max_address_under_test; // byte addressable;
#pragma HLS reset variable=max_address_under_test
#pragma HLS reset variable=outNetMeta
#pragma HLS reset variable=processingFSM
static size_t bytes_sent_for_tx =0;


    static local_mem_word_t local_under_test_memory [LOCAL_MEM_ADDR_SIZE];
    static local_mem_addr_non_byteaddressable_t local_mem_addr_non_byteaddressable;
    static local_mem_addr_t curr_address_under_test;
    static int writingCounter;
    static int readingCounter;
    static ap_uint<32> testCounter;
    static ap_uint<counter_width> reading_cntr = 0;
    static ap_uint<counter_width> writing_cntr = 0;


    local_mem_word_t testingVector;
    local_mem_word_t goldenVector;


#pragma HLS reset variable=local_under_test_memory
#pragma HLS reset variable=local_mem_addr_non_byteaddressable
#pragma HLS reset variable=curr_address_under_test
#pragma HLS reset variable=address_under_test
#pragma HLS reset variable=writingCounter
#pragma HLS reset variable=readingCounter
#pragma HLS reset variable=testCounter
//#pragma HLS reset variable=testingVector

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
        curr_address_under_test = 0; 
        first_faulty_address = 0; 
        faulty_addresses_cntr = 0;
        local_mem_addr_non_byteaddressable = 0;
        readingCounter=0;
        writingCounter=0;
        testCounter = 0;
        processingFSM = FSM_PROCESSING_DATAFLOW_WRITE;//FSM_PROCESSING_WRITE;
        break;

  //run continuously, hence more than once
      case FSM_PROCESSING_CONTINUOUS_RUN:
        testCounter += 1;
        curr_address_under_test = 0; 
        local_mem_addr_non_byteaddressable = 0;
        readingCounter=0;
        writingCounter=0;
        faulty_addresses_cntr = 0;

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
        processingFSM = FSM_PROCESSING_DATAFLOW_WRITE;//FSM_PROCESSING_WRITE;

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
    pWriteDataflowMemTest( lcl_mem0, max_address_under_test, &writing_cntr,&testCounter);
      processingFSM = FSM_PROCESSING_DATAFLOW_READ;
      break;

      case FSM_PROCESSING_DATAFLOW_READ:
    #if DEBUG_LEVEL == TRACE_ALL
      printf("DEBUG processing read dataflow\n");
    #endif
      pReadDataflowMemTest(lcl_mem1,max_address_under_test,&reading_cntr,&faulty_addresses_cntr, &first_faulty_address);
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
          // outNetWord.tdata.range(64-1,64-32) = writing_cntr;
          // outNetWord.tdata.range(32-1,0) = reading_cntr;
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

#endif //_ROLE_MEMTEST_PATTERN_H_
