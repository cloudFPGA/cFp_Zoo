
#ifndef _ROLE_MEMTEST_PATTERN_H_
#define _ROLE_MEMTEST_PATTERN_H_

#include <stdio.h>
#include <iostream>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>

#include "network.hpp"

using namespace hls;

#define FAULT_INJECTION // macro for fault injection insertion

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
//const char * lorem_ipsum_pattern = "Lorem ipsum dolor sit amet, consectetur adipiscing elit,sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.Feugiat in fermentum posuere urna.Purus semper eget duis at tellus at urna condimentum mattis.Euismod nisi porta lorem mollis aliquam ut porttitor leo a. Dolor purus non enim praesent elementum facilisis. Ultrices gravida dictum fusce ut placerat orci.Egestas integer eget aliquet nibh. Pharetra pharetra massa massa ultricies mi quis hendrerit dolor.Ut tortor pretium viverra suspendisse potenti. Mattis aliquam faucibus purus in massa tempor nec feugiat. Tellus integer feugiat scelerisque varius morbi enim nunc faucibus a.Ipsum dolor sit amet consectetur adipiscing elit ut aliquam. Lacinia at quis risus sed vulputate odio ut enim blandit. Facilisi morbi tempus iaculis urna id.Elementum facilisis leo vel fringilla est ullamcorper eget.Dolor sed viverra ipsum nunc.Non tellus orci ac auctor augue mauris augue neque. Sit amet facilisis magna etiam tempor orci eu lobortis.Pellentesque diam volutpat commodo sed egestas egestas fringilla phasellus.Ac felis donec et odio pellentesque diam volutpat. In ornare quam viverra orci sagittis eu volutpat. Rhoncus mattis rhoncus urna neque. Proin sagittis nisl rhoncus mattis rhoncus urna neque viverra justo.Morbi blandit cursus risus at ultrices mi tempus imperdiet nulla.Eu sem integer vitae justo eget magna fermentum iaculis eu.Risus quis varius quam quisque id diam vel. Sem viverra aliquet eget sit amet tellus cras.Ut tristique et egestas quis ipsum suspendisse ultrices gravida. Semper risus in hendrerit gravida rutrum quisque non tellus orci.Massa placerat duis ultricies lacus sed turpis. Sem fringilla ut morbi tincidunt augue interdum velit.Vitae congue mauris rhoncus aenean vel elit.Eu lobortis elementum nibh tellus molestie nunc non blandit massa.Nibh sit amet commodo nulla facilisi nullam vehicula. In fermentum et sollicitudin ac orci phasellus egestas tellus rutrum. Ut sem nulla pharetra diam sit amet nisl suscipit.Quis enim lobortis scelerisque fermentum dui. Lacus suspendisse faucibus interdum posuere lorem ipsum dolor sit amet.Enim tortor at auctor urna nunc. Sed arcu non odio euismod lacinia at. Natoque penatibus et magnis dis parturient montes nascetur.Mus mauris vitae ultricies leo integer malesuada. Viverra tellus in hac habitasse platea dictumst vestibulum rhoncus est.Facilisis gravida neque convallis a cras semper. Imperdiet nulla malesuada pellentesque elit eget gravida. Risus nec feugiat in fermentum posuere.Turpis tincidunt id aliquet risus. Felis imperdiet proin fermentum leo vel orci porta.Tristique senectus et netus et malesuada fames ac turpis egestas.Arcu dictum varius duis at consectetur lorem. Tristique magna sit amet purus gravida quis blandit.Sapien nec sagittis aliquam malesuada bibendum arcu vitae elementum.Commodo viverra maecenas accumsan lacus. Arcu ac tortor dignissim convallis aenean et.Integer feugiat scelerisque varius morbi enim nunc. Tellus integer feugiat scelerisque varius morbi enim nunc faucibus.In egestas erat imperdiet sed euismod nisi. Cursus metus aliquam eleifend mi in nulla posuere sollicitudin aliquam.Parturient montes nascetur ridiculus mus mauris vitae ultricies. Eros donec ac odio tempor orci.Enim facilisis gravida neque convallis a cras semper auctor. Odio pellentesque diam volutpat commodo.Volutpat commodo sed egestas egestas fringilla phasellus.Neque egestas congue quisque egestas diam in arcu cursus euismod.Adipiscing diam donec adipiscing tristique risus. Quis auctor elit sed vulputate mi sit amet mauris commodo.Quis blandit turpis cursus in hac habitasse platea dictumst. Praesent semper feugiat nibh sed pulvinar.Lorem mollis aliquam ut porttitor leo a. Sollicitudin nibh sit amet commodo nulla facilisi nullam vehicula ipsum.Tortor dignissim convallis aenean et tortor at risus viverra adipiscing. Ac turpis egestas sed tempus urna et.Lectus proin nibh nisl condimentum id venenatis a condimentum vitae. Tempus egestas sed sed risus pretium quam.Tortor dignissim convallis aenean et tortor. Aliquet nec ullamcorper sit amet risus.Faucibus in ornare quam viverra orci sagittis eu volutpat odio. Quis eleifend quam adipiscing vitae proin sagittis nisl.Vestibulum rhoncus est pellentesque elit ullamcorper dignissim cras.Morbi tincidunt augue interdum velit euismod in pellentesque massa placerat.Pellentesque massa placerat duis ultricies lacus sed turpis tincidunt.Sit amet nisl purus in mollis. Posuere sollicitudin aliquam ultrices sagittis orci a.\0";
#define LOCAL_MEM_WORD_SIZE 512
#define LOCAL_MEM_ADDR_SIZE 20
typedef ap_uint<LOCAL_MEM_WORD_SIZE>  local_mem_word_t;
typedef ap_uint<32>  local_mem_addr_t; 
#define LOCAL_MEM_ADDR_SIZE_NON_BYTE_ADDRESSABLE 16
typedef ap_uint<LOCAL_MEM_ADDR_SIZE_NON_BYTE_ADDRESSABLE>  local_mem_addr_non_byteaddressable_t; 
#define LOCAL_MEM_ADDR_OFFSET (LOCAL_MEM_WORD_SIZE/8) //byte addres offset
#define LOCAL_MEM_WORD_BYTE_SIZE (LOCAL_MEM_WORD_SIZE/8) //byte addres offset

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
    //printf("MEMTEST PATTERN %d %d\n", smallWordDim*(i+1)-1,smallWordDim*i);
    // printf("MEMTEST PATTERN Next fibo number %u\n", nextFibonacciNumber.to_string());
    //   std'::cout << "MEMTEST PATTERN Next fibo number " << nextFibonacciNumber << std::endl;
    (*outBigWord).range(smallWordDim*(i+1)-1,smallWordDim*i)=nextFibonacciNumber;
    prevFibonacciNumber=currentFibonacciNumber;
    currentFibonacciNumber=nextFibonacciNumber;
    nextFibonacciNumber=genNextFibonacciNumber<ADDR_T,SMALLWORD_T>(currentFibonacciNumber,prevFibonacciNumber);
    #ifndef __SYNTHESIS__
    std::cout << "MEMTEST PATTERN prev " << prevFibonacciNumber << " curr " << currentFibonacciNumber << "  next " << nextFibonacciNumber << std::endl;
    #endif //__SYNTHESIS__
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
    //printf("MEMTEST PATTERN %d %d\n", smallWordDim*(i+1)-1,smallWordDim*i);
    // printf("MEMTEST PATTERN Next fibo number %u\n", nextFibonacciNumber.to_string());
    //   std'::cout << "MEMTEST PATTERN Next fibo number " << nextFibonacciNumber << std::endl;
    (*outBigWord).range(smallWordDim*(i+1)-1,smallWordDim*i)=nextNumber;
    prevNumber = currentNumber;
    currentNumber = nextNumber;
    nextNumber = (nextNumber + 1 ) xor i;
    // #ifndef __SYNTHESIS__
    //  std::cout << "MEMTEST PATTERN prev " << prevNumber << " curr " << currentNumber << "  next " << nextNumber << " \t";
   //  std::cout << "MEMTEST PATTERN curr " << currentNumber << " \t";
    // #endif //__SYNTHESIS__
  }
 //    std::cout << std::endl;
}

#endif //_ROLE_MEMTEST_PATTERN_H_
