  
#ifndef _ROLE_MEMTEST_PATTERN_H_
#define _ROLE_MEMTEST_PATTERN_H_

#include "../include/memtest_library.hpp"

#define FAULT_INJECTION // macro for fault injection insertion

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


#define LOCAL_MEM_WORD_SIZE 512
#define LOCAL_MEM_ADDR_SIZE 20
#define MEMTEST_ADDRESS_BITWIDTH 40
#define MEMTEST_ITERATION_BITWIDTH 16
#define MEMTEST_BURST_BITWIDTH 8


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



#define MEMTEST_BURST_HIGH_BIT 16-1 // 15
#define MEMTEST_BURST_LOW_BIT  MEMTEST_BURST_HIGH_BIT+1-MEMTEST_BURST_BITWIDTH // 8

#define MAX_ITERATION_COUNT 10
const unsigned int max_proc_fifo_depth = MAX_ITERATION_COUNT;

//////////////////////////////////////////////////////////////////////////////
//////////////////Begin of Generate Functions/////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

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
void genXoredNumbersSingleWord(ADDR_T curr, BIGWORD_T * outBigWord){
#pragma HLS INLINE 
  BIGWORD_T currentNumber = static_cast<BIGWORD_T>(curr);
  BIGWORD_T nextNumber = (currentNumber+1) xor 1;
  *outBigWord=nextNumber;
}


template<typename ADDR_T, typename BIGWORD_T>
void genSequentialNumbers(ADDR_T curr, BIGWORD_T * outBigWord){
#pragma HLS INLINE 
  *outBigWord = curr+1;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////End of Generate Functions///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//////////////////Begin of WriteFunctions/////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

template<const unsigned int max_iterations=4000000, const unsigned int buff_dim = 16>
void pWRGenerateData2WriteOnStream(
  hls::stream<local_mem_word_t>& generatedData,
  ap_uint<32> * testCounter,
  local_mem_addr_t max_addr_ut)
{
#pragma HLS INLINE off
    static local_mem_word_t tmp_out [buff_dim];
#pragma HLS array_partition variable=tmp_out cyclic factor=2 dim=1

    //local_mem_addr_non_byteaddressable_t maddr_non_byte;
    static local_mem_addr_t curr_address_ut=0;
  generate_loop:
  for (; curr_address_ut < max_addr_ut; )
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
//if free to generate or need backpressure
    if (!generatedData.full())
    {
      //Step 1) Generated the data according to a function

      //  genXoredSequentialNumbersSecondVersion
      //  <local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32, local_mem_word_t,
      //   ap_uint<32>,32>(maddr_non_byte, 
      //    tmp_out+(maddr_non_byte));
      //genSequentialNumbers<local_mem_addr_non_byteaddressable_t,local_mem_word_t>(curr_address_ut*64, tmp_out+(maddr_non_byte));
      genXoredNumbersSingleWord<local_mem_addr_t,
      local_mem_word_t>(curr_address_ut, tmp_out+(curr_address_ut%buff_dim));

//Step 2) Optional fault injection
      local_mem_word_t tmp_out_scalar;
      #ifdef FAULT_INJECTION
          //TODO:  place for control fault injection with a function?
          if(*testCounter >= 2 && curr_address_ut > 0){
              tmp_out_scalar = tmp_out[curr_address_ut%buff_dim] & static_cast<local_mem_word_t>(0);
            }else{
              tmp_out_scalar = tmp_out[curr_address_ut%buff_dim];
            }
      #else // FAULT_INJECTION
        tmp_out_scalar = tmp_out[curr_address_ut%buff_dim];
      #endif // FAULT_INJECTION
      //Step 3) write out to the next macrostep and going to the next element
      generatedData.write(tmp_out_scalar);
      curr_address_ut++;
    }
    //else account for a newer cycle to write
  }
  curr_address_ut=0;
  

}


template <typename Tcntr, const unsigned int max_iterations=4000000,
const unsigned int buff_dim=64*2>
void pWRStream2WriteMainMemory(
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
    static bool activated_cntr = false;
#pragma HLS array_partition variable=tmp_out block factor=2 dim=1

  // cmd.write(0);
  int idx, written_i;
  int ptrs_difference=0;
  unsigned int last_words=0;
  read_and_write:
  for (curr_address_ut = 0, idx=0, curr_writing_addr=0, written_i=0; curr_address_ut < max_addr_ut; curr_address_ut++)// curr_address_ut+=LOCAL_MEM_ADDR_OFFSET)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    if (!generatedData.empty())
    {
      tmp_out[idx] = generatedData.read();
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
      std::cout << tmp_out[idx] << std::endl;
      #endif//synth
      #endif//debug lvl
      //if stored enough data to begin the bursting OR this is last iteration
      end_distance = max_addr_ut-curr_address_ut;
      //last_iteration = burst_size>end_distance+1;
      //last_iteration = burst_size>end_distance;
      last_iteration = 1>=end_distance;
      //ptrs_difference = idx-written_i;
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
      std::cout << "test addr " << idx << " current address " << curr_address_ut << " max addr " << max_addr_ut << std::endl;
      std::cout << "ptrs_difference " << ptrs_difference << " last_iteration " << last_iteration <<std::endl;
      #endif
      #endif
//accumulated a burst or last iteration
      //if ((ptrs_difference>0 && ptrs_difference>=burst_size-1) || (ptrs_difference<0 && ptrs_difference<1-burst_size) || last_iteration)
      if ((ptrs_difference>0 && ptrs_difference>=burst_size-1) || (last_iteration))
      //if ((ptrs_difference>0 && ptrs_difference>=burst_size-1) || (last_iteration && ptrs_difference>=end_distance-1))
      {
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
      std::cout << "Burst filled or last iteration, end distance will be= " << end_distance << std::endl;
      #endif
      #endif
        if (!last_iteration)
        {
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << "BURST transferring " << burst_size << " words at " << curr_writing_addr << " address, from  " << written_i << std::endl;
      #endif
      #endif
          if(!activated_cntr){
            cmd.write(0);
            activated_cntr = true;
          }else{
            cmd.write(1);
          }
          memcpy(lcl_mem+curr_writing_addr, tmp_out+written_i, sizeof(local_mem_word_t)*burst_size);
          cmd.write(1);
          curr_writing_addr+=burst_size;
          written_i= (written_i+burst_size)%buff_dim;
          ptrs_difference-=burst_size;
          last_words=end_distance;
          //ptrs_difference = idx-written_i;
        }else{
          //unsigned int last_words=(idx%burst_size+1);
          //unsigned int last_words=end_distance;//(idx%burst_size+1);
      #if DEBUG_LEVEL == TRACE_ALL
      //#ifndef __SYNTHESIS__
          std::cout << "LAST transferring " << last_words << " words at " << curr_writing_addr << " address, from  " << written_i << std::endl;
      //#endif
      #endif
          if(!activated_cntr){
            cmd.write(0);
            activated_cntr = true;
          }else{
            cmd.write(1);
          }
          memcpy(lcl_mem+curr_writing_addr, tmp_out+written_i, sizeof(local_mem_word_t)*(last_words));
          cmd.write(1);
          curr_writing_addr+=(last_words);
          written_i=(written_i+last_words)%buff_dim;
          ptrs_difference-=last_words;
          //ptrs_difference = idx-written_i;
        }
      }
      if(idx==buff_dim-1){
        idx=0;
      }else{
        idx++;
      }
      ptrs_difference++;
     // std::cout << std::endl;
     // memcpy(lcl_mem+curr_address_ut, tmp_out+curr_address_ut%buff_dim, sizeof(local_mem_word_t));
    }else{
      curr_address_ut--;
    }
  }
    cmd.write(0);//quit everything
    end_distance=0;
    last_iteration = false;
}


template <typename Tcntr, const unsigned int max_iterations=4000000,const unsigned int buff_dim=64*2>
void pWriteStupidTestMemTest(
hls::stream<Tcntr>& cmd,
membus_t * lcl_mem,
local_mem_addr_t max_addr_ut,
unsigned int burst_size)
{
  cmd.write(0);
  local_mem_addr_t curr_address_ut;
  for (curr_address_ut = 0; curr_address_ut < max_addr_ut; curr_address_ut++)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
    printf("Writing address %s with max %s\n",curr_address_ut.to_string().c_str(), max_addr_ut.to_string().c_str());
    #endif
    #endif
    lcl_mem[curr_address_ut]=curr_address_ut+1;
  }
  cmd.write(0);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////End of Write Functions//////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//////////////////Begin of Read Functions/////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


template <typename Tcntr, const unsigned int max_iterations=4000000,const unsigned int buff_dim=64*2>
void pReadStupidTestMemTest(
hls::stream<Tcntr>& cmd,
membus_t * lcl_mem,
local_mem_addr_t max_addr_ut,
unsigned int burst_size ,
ap_uint<32> * faulty_addresses_cntr,
local_mem_addr_t * first_faulty_address)
{
  local_mem_addr_t curr_address_ut;
  int faults = 0;
  cmd.write(0);
  for (curr_address_ut = 0; curr_address_ut < max_addr_ut; curr_address_ut++){
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
  printf("Tst=%s vs gld=%s\n",lcl_mem[curr_address_ut].to_string().c_str(),(curr_address_ut+1).to_string().c_str());
  #endif 
  #endif
    faults +=  (lcl_mem[curr_address_ut]!=(curr_address_ut+1)) ? 1 : 0;
  }
  cmd.write(0);
  *faulty_addresses_cntr = faults;
  *first_faulty_address = 0;
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
#pragma HLS array_partition variable=tmp_out cyclic factor=2 dim=1

  cmd.write(0);
  int i, reading_i;
  read_data_from_main_mem:
  for (i = 0, curr_address_ut = 0, curr_reading_addr=0, reading_i=0; curr_address_ut < max_addr_ut; )// curr_address_ut+=LOCAL_MEM_ADDR_OFFSET, i++)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    if (!readData.full())
    {

      memcpy(tmp_out+reading_i, lcl_mem+curr_address_ut, sizeof(local_mem_word_t));
      if(reading_i > curr_reading_addr+1){
        readData.write(tmp_out[curr_reading_addr]);
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
        std::cout << " writing a memory word " << curr_reading_addr << " I have to reach " << i << std::endl;
      #endif
      #endif
        curr_reading_addr++;

      }
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
      std::cout << "read the " << reading_i << " memory word, outputreading at  " << curr_reading_addr << " i at " << i << std::endl;
      #endif
      #endif

      if(reading_i==buff_dim-1){
        reading_i=0;
      }else{
        reading_i++;
      }
      curr_address_ut++;
      i++;
    }
  }

  cmd.write(0);
  i--;
  reading_i=i%buff_dim;
  sent_out_remaining_buff: 
  for (int j = 0; j < buff_dim; j++)
  {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = buff_dim
#pragma HLS PIPELINE II=1
    if (!readData.full())
    {
      if(j==curr_reading_addr && i >= curr_reading_addr){
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
        std::cout << " writing a memory word " << curr_reading_addr << " I have to reach " << i << std::endl;
      #endif
      #endif
        readData.write(tmp_out[curr_reading_addr]);
        curr_reading_addr++;
        tmp_out[j]=0;
      }else{
        tmp_out[j]=0;
      }
    }else{
      j--;
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

    static local_mem_addr_t curr_address_ut= 0;
    local_mem_word_t testingVector;
    local_mem_word_t goldenVector;

  generate_loop:
  for (; curr_address_ut < max_addr_ut; )// curr_address_ut+=LOCAL_MEM_ADDR_OFFSET)
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    if (!readData.empty() && !outReadData.full() &&  !outGoldData.full())
    {
      testingVector = readData.read(); 

      // genXoredSequentialNumbersSecondVersion<local_mem_addr_non_byteaddressable_t, LOCAL_MEM_WORD_SIZE/32,
      //  local_mem_word_t, ap_uint<32>,32>(local_mem_addr_non_byteaddressable, &goldenVector);
      genXoredNumbersSingleWord<local_mem_addr_t,
      local_mem_word_t>(curr_address_ut, &goldenVector);
      //genSequentialNumbers<local_mem_addr_non_byteaddressable_t,local_mem_word_t>(curr_address_ut*64, &goldenVector);

      outReadData.write(testingVector);
      outGoldData.write(goldenVector);
      curr_address_ut++;
    }
  }
  curr_address_ut = 0;
}

template <typename Tcntr, const unsigned int max_iterations=4000000,
const unsigned int buff_dim=64*2>
void pRDRead2StreamDataVariableBurst(
  hls::stream<Tcntr>& cmd,
  hls::stream<local_mem_word_t>& readData,
  membus_t * lcl_mem,
  local_mem_addr_t max_addr_ut,
unsigned int burst_size)
{
#pragma HLS INLINE off

    local_mem_addr_t curr_address_ut;
    static local_mem_word_t tmp_out[buff_dim];
#pragma HLS array_partition variable=tmp_out cyclic factor=2 dim=1
////TODO: Check if with 64 is better
    static local_mem_addr_t curr_reading_addr;
    static unsigned int end_distance=0;
    static int ptrs_distance=0;
    static int ptrs_distance_opposite=0;
    static bool transfer_less_than_burst = false;
    static bool activated_cntr = false;
    static bool can_read_data = false;

  read_data_from_main_mem:
  int reading_mm_i = 0;//how much filled the buff
  int consumed_fifo_i = 0;//how much already outputed
  unsigned int total_consumed_words=0;
  unsigned int total_readfrom_mm_words=0;
  bool fifo_is_not_full=false;
  for (curr_address_ut = 0, curr_reading_addr=0; curr_address_ut < max_addr_ut; )
  {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    if (!readData.full())
    {

      end_distance = max_addr_ut-curr_reading_addr;
      ptrs_distance = total_readfrom_mm_words - total_consumed_words;
      ptrs_distance_opposite = total_consumed_words - total_readfrom_mm_words;
      // ptrs_distance =reading_mm_i-consumed_fifo_i;
      transfer_less_than_burst = burst_size>end_distance;
      //fifo_is_not_full = (reading_mm_i+burst_size)%buff_dim>consumed_fifo_i;
      fifo_is_not_full = ptrs_distance <= burst_size;
      //can_read_data=(end_distance > 0) && ((ptrs_distance<=burst_size || -1*ptrs_distance>burst_size) || transfer_less_than_burst);
      //i have data to crunch, and eithre the fifo can read a burst or consumed enough to tx less than a burst)
      can_read_data=(end_distance > 0) && (fifo_is_not_full  || (ptrs_distance<=end_distance && transfer_less_than_burst));
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << "Max " << max_addr_ut << " reading at " << curr_reading_addr << " reading fifo at " << reading_mm_i <<  " consumed at " << consumed_fifo_i << std::endl;
          std::cout << "End dst " << end_distance << " ptrs dst " << ptrs_distance << " is last? " << transfer_less_than_burst << std::endl;
          std::cout << "curr_address_ut " << curr_address_ut << std::endl;  
      #endif
      #endif
      //if more than a burst size to available or the last iteration
      if(can_read_data){
        if (!transfer_less_than_burst)
        {
          //read a burst
          if(!activated_cntr){
            cmd.write(0);
            activated_cntr = true;
          }else{
            cmd.write(1);
          }
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << "BURST reading " << burst_size << " words from " << curr_reading_addr << " address, to  " << reading_mm_i << std::endl;
      #endif
      #endif
          memcpy(tmp_out+reading_mm_i, lcl_mem+curr_reading_addr, sizeof(local_mem_word_t)*burst_size);
          cmd.write(1);
          curr_reading_addr+=burst_size;
          total_readfrom_mm_words+=burst_size;
          reading_mm_i=(reading_mm_i+burst_size)%buff_dim;
        }else{
          //read the missing words
          if(!activated_cntr){
            cmd.write(0);
            activated_cntr = true;
          }else{
            cmd.write(1);
          }
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << "LAST reading " << end_distance%burst_size << " words from " << curr_reading_addr << " address, to  " << reading_mm_i << std::endl;
      #endif
      #endif
          total_readfrom_mm_words+=end_distance%burst_size;
          memcpy(tmp_out+reading_mm_i, lcl_mem+curr_reading_addr, sizeof(local_mem_word_t)*(end_distance%burst_size));
          cmd.write(1);
          curr_reading_addr+=(end_distance%burst_size);
          reading_mm_i=(reading_mm_i+end_distance%burst_size)%buff_dim;
        }
      }

      if(ptrs_distance > 0 || can_read_data){
        readData.write(tmp_out[consumed_fifo_i]);
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
        std::cout << " consumin a read memory word " << consumed_fifo_i << " I have to reach " << reading_mm_i << std::endl;
        std::cout << " The readmemoryword is  " << tmp_out[consumed_fifo_i]  << std::endl;
      #endif
     #endif

        if(consumed_fifo_i==buff_dim-1){
          consumed_fifo_i=0;
        }else{
          consumed_fifo_i++;
        }
        total_consumed_words++;
      }
      //// TODO: add this logic to end before the loop
      //// Example: with burst 64 the loop need less CC to consume everything
      //if() still data to read and to consume
      curr_address_ut++;
      //else{
      //  curr_address_ut=max_addr_ut;
      //}
    }
  }
  cmd.write(0);
  //reset
  end_distance=0;
  transfer_less_than_burst = false;
  activated_cntr=false;
  can_read_data=false;
  ptrs_distance=0;
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
  for (curr_address_ut = 0, k=0, faulty_addresses_cntr_local=0; curr_address_ut < max_addr_ut; k++, curr_address_ut++)// curr_address_ut+=LOCAL_MEM_ADDR_OFFSET, k++)
  {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
    if (!sInReadData.empty() && !sInGoldData.empty())
    {
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
        //#if DEBUG_LEVEL == TRACE_ALL
        #ifndef __SYNTHESIS__
            std::cout << " tst=" << testingVector_bytes[idx] << " vs gld=" << goldenVector_bytes[idx] ;
        #endif
        //#endif

          cmp_ok[idx] = testingVector_bytes[idx] == goldenVector_bytes[idx];
         // tmpOut[i] = !cmp_ok[idx];
          if(!cmp_ok[idx] ){
            faulty_addresses_cntr_support_array[i+k]++;
            if (!first_fault_found)
            {
              *first_faulty_address=i+curr_address_ut*64;
              first_fault_found = true;
            }else{
              first_fault_found = first_fault_found;
            }
          }
       }
        #if DEBUG_LEVEL == TRACE_ALL
        #ifndef __SYNTHESIS__
       std::cout << std::endl;
       #endif
       #endif
       //sInCmpRes.write(tmpOut);
        maddr_non_byte++;
    }else{
      k--;
      curr_address_ut--;
    }
  }

  flt_cntr_loop: for (int i = 0; i < support_dim; i++)
  {
#pragma HLS PIPELINE
    faulty_addresses_cntr_local += faulty_addresses_cntr_support_array[i];
    faulty_addresses_cntr_support_array[i]=0;
  }
  
  first_fault_found=false;
*faulty_addresses_cntr=faulty_addresses_cntr_local;

}

template<const unsigned int max_iterations=4000000, const unsigned int buff_dim=16>
void pRDCmpStreamsCntWordAligned(
  local_mem_addr_t max_addr_ut,
  hls::stream<local_mem_word_t>& sInReadData,
  hls::stream<local_mem_word_t>& sInGoldData,
  ap_uint<32> * faulty_addresses_cntr,
  local_mem_addr_t * first_faulty_address)
{
//#pragma HLS INLINE off
    local_mem_addr_t curr_address_ut;
    static local_mem_word_t testingVector[buff_dim];
    static local_mem_word_t goldenVector[buff_dim];
    local_mem_addr_non_byteaddressable_t maddr_non_byte=0;
    static bool cmp_ok [buff_dim];
    static ap_uint<32> faulty_addresses_cntr_support_array [buff_dim];

// #pragma HLS array_partition variable=faulty_addresses_cntr_support_array cyclic factor=2 dim=1
// #pragma HLS array_partition variable=testingVector_bytes cyclic factor=2 dim=1
// #pragma HLS array_partition variable=goldenVector_bytes cyclic factor=2 dim=1
// #pragma HLS array_partition variable=cmp_ok cyclic factor=2 dim=1
    static bool first_fault_found  = false;
    static ap_uint<32> faulty_addresses_cntr_local;

      reading_loop:
  for (curr_address_ut = 0, faulty_addresses_cntr_local=0; curr_address_ut < max_addr_ut; curr_address_ut++)// curr_address_ut+=LOCAL_MEM_ADDR_OFFSET, k++)
  {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = 1 max = max_iterations
#pragma HLS dependence variable=faulty_addresses_cntr_support_array inter RAW distance=16 true

    if (!sInReadData.empty() && !sInGoldData.empty())
    {
      testingVector[maddr_non_byte%buff_dim] = sInReadData.read(); 
      goldenVector[maddr_non_byte%buff_dim] = sInGoldData.read(); 

      int idx=maddr_non_byte%buff_dim;
      #if DEBUG_LEVEL == TRACE_ALL
      #ifndef __SYNTHESIS__
          std::cout << maddr_non_byte << " tst=" << testingVector[idx] << " vs gld=" << goldenVector[idx] << std::endl;
      #endif
      #endif
      bool cmp_results = testingVector[idx] == goldenVector[idx];
      cmp_ok[idx]=cmp_results;
      if(!cmp_results ){
        faulty_addresses_cntr_support_array[idx]++;
        if (!first_fault_found)
        {
          *first_faulty_address=curr_address_ut*64;
          first_fault_found = true;
        }else{
          first_fault_found = first_fault_found;
        }
      }

        maddr_non_byte++;
    }else{
      curr_address_ut--;
    }
  }
 // std::cout << std::endl;

  for (int i = 0; i < buff_dim; i++)
  {
#pragma HLS PIPELINE
    faulty_addresses_cntr_local += faulty_addresses_cntr_support_array[i];
    faulty_addresses_cntr_support_array[i]=0;
  }
  
  first_fault_found=false;
*faulty_addresses_cntr=faulty_addresses_cntr_local;

}

//////////////////////////////////////////////////////////////////////////////
//////////////////End of Read/ Functions//////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//////////////////Begin of WR/RD Composed Functions///////////////////////////
//////////////////////////////////////////////////////////////////////////////

void pWriteDataflowMemTest(
  membus_t * lcl_mem0,
  local_mem_addr_t max_address_under_test,
  ap_uint<64> * writing_cntr,
  ap_uint<32> * testCounter,
  unsigned int burst_size)
{
  #pragma HLS INLINE off
     static hls::stream<ap_uint<64>> sWritePrfCntr_cmd("sWritePrfCntr_cmd"); 
     #pragma HLS STREAM variable=sWritePrfCntr_cmd depth=64 dim=1
     static hls::stream<local_mem_word_t> generatedWriteData("generatedWriteData"); 
     #pragma HLS STREAM variable=generatedWriteData depth=64 dim=1
    #pragma HLS DATAFLOW
          //Step 0  for debugging
          //pWriteStupidTestMemTest<ap_uint<64>>(sWritePrfCntr_cmd, lcl_mem0, max_address_under_test, burst_size);
          //Step 1: Generate the data
          pWRGenerateData2WriteOnStream<4000000>(generatedWriteData,testCounter,max_address_under_test);
          //Step 2: write 
          pWRStream2WriteMainMemory<ap_uint<64>,4000000>(sWritePrfCntr_cmd, generatedWriteData, lcl_mem0, max_address_under_test, burst_size);
          //Step 2.b: count 
          perfCounterMultipleCounts<ap_uint<64>,ap_uint<64>,64>(sWritePrfCntr_cmd, writing_cntr);
          //perfCounterProc2MemCountOnly<ap_uint<64>,ap_uint<64>,64>(sWritePrfCntr_cmd, writing_cntr);
}


void pReadDataflowMemTest(
  membus_t * lcl_mem1,
  local_mem_addr_t max_address_under_test,
  ap_uint<64> * reading_cntr,
  ap_uint<32> * faulty_addresses_cntr,
  local_mem_addr_t * first_faulty_address,
  unsigned int burst_size)
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
      //Step 0  for debugging
      //pReadStupidTestMemTest<ap_uint<64>>(sReadPrfCntr_cmd, lcl_mem1, max_address_under_test, burst_size, faulty_addresses_cntr, first_faulty_address);

      //Step 1: Generate the data
      //pRDMainMemoryRead2StreamData<ap_uint<64>,4000000>( sReadPrfCntr_cmd, generatedReadData, lcl_mem1, max_address_under_test,burst_size);
      pRDRead2StreamDataVariableBurst<ap_uint<64>,4000000>( sReadPrfCntr_cmd, generatedReadData, lcl_mem1, max_address_under_test,burst_size);
      //Step 2: write 
      pRDReadDataStreamAndProduceGold<4000000>(generatedReadData, max_address_under_test, sReadData, sGoldData); 
      //Step 2.b: count 
      //perfCounterProc2MemCountOnly<ap_uint<64>,ap_uint<64>,64>(sReadPrfCntr_cmd, reading_cntr);
      perfCounterMultipleCounts<ap_uint<64>,ap_uint<64>,64>(sReadPrfCntr_cmd, reading_cntr);
      //Step 3: compare
      //pRDCompareDataStreamsCount<4000000>(max_address_under_test,sReadData, sGoldData,faulty_addresses_cntr, first_faulty_address);
      pRDCmpStreamsCntWordAligned<4000000>(max_address_under_test,sReadData, sGoldData,faulty_addresses_cntr, first_faulty_address);
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


    static local_mem_addr_non_byteaddressable_t local_mem_addr_non_byteaddressable;
    static local_mem_addr_t curr_address_under_test;
    static int writingCounter;
    static int readingCounter;
    static ap_uint<32> testCounter;
    static ap_uint<counter_width> reading_cntr = 0;
    static ap_uint<counter_width> writing_cntr = 0;
    static unsigned int burst_size=1;
    
    static local_mem_addr_t tmp_wordaligned_address = 0;

    local_mem_word_t testingVector;
    local_mem_word_t goldenVector;

static int emptycntr=0;

#pragma HLS reset variable=local_mem_addr_non_byteaddressable
#pragma HLS reset variable=curr_address_under_test
#pragma HLS reset variable=address_under_test
#pragma HLS reset variable=writingCounter
#pragma HLS reset variable=readingCounter
#pragma HLS reset variable=testCounter
#pragma HLS reset variable=tmp_wordaligned_address
#pragma HLS reset variable=burst_size
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
          local_mem_addr_non_byteaddressable = 0;
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
        std::cout << netWord.tdata.to_string() << std::endl;
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
        curr_address_under_test = 0; 
        first_faulty_address = 0; 
        faulty_addresses_cntr = 0;
       // local_mem_addr_non_byteaddressable = max_address_under_test%64==0 ? (unsigned int)max_address_under_test/64 : ((unsigned int)(max_address_under_test/64))+1;//word align
       if(max_address_under_test%64==0){
          tmp_wordaligned_address = max_address_under_test/64;
       } else {
         tmp_wordaligned_address = (max_address_under_test/64+1);
       }
        //local_mem_addr_non_byteaddressable = tmp_wordaligned_address;//word align
    #if DEBUG_LEVEL == TRACE_ALL
    #ifndef __SYNTHESIS__
        std::cout << " testing the address word aligned" << tmp_wordaligned_address.to_string() << std::endl;
    #endif
    #endif
        readingCounter=0;
        writingCounter=0;
        reading_cntr = 0;
        writing_cntr = 0;
        testCounter = 0;
        processingFSM = FSM_PROCESSING_DATAFLOW_WRITE;//FSM_PROCESSING_WRITE;
        break;

  //run continuously, hence more than once
      case FSM_PROCESSING_CONTINUOUS_RUN:
        testCounter += 1;
        curr_address_under_test = 0; 
        //local_mem_addr_non_byteaddressable = 0;
        readingCounter=0;
        writingCounter=0;
        reading_cntr = 0;
        writing_cntr = 0;
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
    pWriteDataflowMemTest( lcl_mem0, tmp_wordaligned_address , &writing_cntr,&testCounter,burst_size);
      processingFSM = FSM_PROCESSING_DATAFLOW_READ;
      break;

      case FSM_PROCESSING_WAIT_FOR_DDR_CONTROLLER_EMPTYNESS:
      #if DEBUG_LEVEL == TRACE_ALL
        printf("DEBUG processing the output of a run\n");
      #endif
        emptycntr++;
        if(emptycntr==53){
          processingFSM = FSM_PROCESSING_DATAFLOW_READ;
          emptycntr=0;
        }
      break;

      case FSM_PROCESSING_DATAFLOW_READ:
#if DEBUG_LEVEL == TRACE_ALL 
      printf("DEBUG processing read dataflow\n");
#endif
      pReadDataflowMemTest(lcl_mem1, tmp_wordaligned_address ,&reading_cntr,&faulty_addresses_cntr, &first_faulty_address,burst_size);
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
