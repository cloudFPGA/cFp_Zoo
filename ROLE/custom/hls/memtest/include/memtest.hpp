/*****************************************************************************
 * @file       memtest.hpp
 * @brief      The Role for a Memtest Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID
 * @date       May 2020
 *----------------------------------------------------------------------------
 *
 * @details    : This application implements a set of UDP-oriented tests and
 *  functions which are embedded into the Flash of the cloudFPGA role.
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


#ifndef _ROLE_MEMTEST_H_
#define _ROLE_MEMTEST_H_

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <math.h>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>

#include "network.hpp"

using namespace hls;


/********************************************
 * SHELL/MMIO/EchoCtrl - Config Register
 ********************************************/
enum EchoCtrl {
	ECHO_PATH_THRU	= 0,
	ECHO_STORE_FWD	= 1,
	ECHO_OFF	= 2
};

/********************************************
 * Internal uppercase accelerator command
 ********************************************/
enum UppercaseCmd {
    STOP_CMD  =0000000000000000,
    START_CMD  =0000000000000001
};


/********************************************
 * Internal MemTest accelerator command
 ********************************************/
enum MemTestCmd {
    TEST_STOP_CMD  = 2,
    TEST_START_CMD = 1,
    TEST_INVLD_CMD = 0
};

#define WAIT_FOR_META 0
#define WAIT_FOR_STREAM_PAIR 1
#define PROCESSING_PACKET 2
#define MEMTEST_RETURN_RESULTS 3

#define PacketFsmType uint8_t

#define FSM_WRITE_NEW_DATA 0
#define FSM_DONE 1
#define PortFsmType uint8_t

#define FSM_PROCESSING_STOP 0
#define FSM_PROCESSING_START 1
#define FSM_PROCESSING_WRITE 2
#define FSM_PROCESSING_READ 3
#define FSM_PROCESSING_OUTPUT 4
#define ProcessingFsmType uint8_t

#define LOCAL_MEM_WORD_SIZE 512;
#define LOCAL_MEM_ADDR_SIZE 20;
typedef ap_uint<LOCAL_MEM_WORD_SIZE>  local_mem_word_t;    // change to float or double depending on your needs
typedef ap_uint<32>  local_mem_addr_t;    // change to float or double depending on your needs
#define LOCAL_MEM_ADDR_OFFSET LOCAL_MEM_WORD_SIZE/8; //byte addres offset
#define LOCAL_MEM_WORD_BYTE_SIZE LOCAL_MEM_WORD_SIZE/8; //byte addres offset


#define DEFAULT_TX_PORT 2718
#define DEFAULT_RX_PORT 2718


#define MEMDW 64          // 512 or 128 or 64 // Bus width in bits for Host memory
#define BPERDW (MEMDW/8)   // Bytes per Data Word    if MEMDW=512 => BPERDW = 64, if MEMDW=64 => BPERDW = 16

#define MAX_NB_OF_ELMT_READ  16
typedef uint8_t  mat_elmt_t; 	// change to float or double depending on your needs

#define MAX_NB_OF_WORDS_READ	(MAX_NB_OF_ELMT_READ*sizeof(mat_elmt_t)/BPERDW) // =2 if double =1 if float
#define MAX_NB_OF_ELMT_PERDW	(BPERDW/sizeof(mat_elmt_t)) // =8 if double =16 if float


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
);


char lorem_ipsum_pattern = "Lorem ipsum dolor sit amet, consectetur adipiscing elit,
sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Feugiat in fermentum posuere urna.
Purus semper eget duis at tellus at urna condimentum mattis.
Euismod nisi porta lorem mollis aliquam ut porttitor leo a. Dolor purus non enim praesent elementum facilisis.
Ultrices gravida dictum fusce ut placerat orci.
Egestas integer eget aliquet nibh. Pharetra pharetra massa massa ultricies mi quis hendrerit dolor. Ut tortor pretium viverra suspendisse potenti. Mattis aliquam faucibus purus in massa tempor nec feugiat. Tellus integer feugiat scelerisque varius morbi enim nunc faucibus a. Ipsum dolor sit amet consectetur adipiscing elit ut aliquam. Lacinia at quis risus sed vulputate odio ut enim blandit.
Facilisi morbi tempus iaculis urna id.
Elementum facilisis leo vel fringilla est ullamcorper eget.
Dolor sed viverra ipsum nunc.
Non tellus orci ac auctor augue mauris augue neque. Sit amet facilisis magna etiam tempor orci eu lobortis. Pellentesque diam volutpat commodo sed egestas egestas fringilla phasellus. Ac felis donec et odio pellentesque diam volutpat. In ornare quam viverra orci sagittis eu volutpat. Rhoncus mattis rhoncus urna neque. Proin sagittis nisl rhoncus mattis rhoncus urna neque viverra justo. Morbi blandit cursus risus at ultrices mi tempus imperdiet nulla. Eu sem integer vitae justo eget magna fermentum iaculis eu. Risus quis varius quam quisque id diam vel. Sem viverra aliquet eget sit amet tellus cras. Ut tristique et egestas quis ipsum suspendisse ultrices gravida. Semper risus in hendrerit gravida rutrum quisque non tellus orci. Massa placerat duis ultricies lacus sed turpis. Sem fringilla ut morbi tincidunt augue interdum velit.
Vitae congue mauris rhoncus aenean vel elit.
Eu lobortis elementum nibh tellus molestie nunc non blandit massa.
Nibh sit amet commodo nulla facilisi nullam vehicula. In fermentum et sollicitudin ac orci phasellus egestas tellus rutrum. Ut sem nulla pharetra diam sit amet nisl suscipit. Quis enim lobortis scelerisque fermentum dui. Lacus suspendisse faucibus interdum posuere lorem ipsum dolor sit amet. Enim tortor at auctor urna nunc. Sed arcu non odio euismod lacinia at. Natoque penatibus et magnis dis parturient montes nascetur. Mus mauris vitae ultricies leo integer malesuada. Viverra tellus in hac habitasse platea dictumst vestibulum rhoncus est. Facilisis gravida neque convallis a cras semper. Imperdiet nulla malesuada pellentesque elit eget gravida. Risus nec feugiat in fermentum posuere.
Turpis tincidunt id aliquet risus. Felis imperdiet proin fermentum leo vel orci porta.
Tristique senectus et netus et malesuada fames ac turpis egestas.
Arcu dictum varius duis at consectetur lorem. Tristique magna sit amet purus gravida quis blandit.
Sapien nec sagittis aliquam malesuada bibendum arcu vitae elementum.
Commodo viverra maecenas accumsan lacus. Arcu ac tortor dignissim convallis aenean et.
Integer feugiat scelerisque varius morbi enim nunc. Tellus integer feugiat scelerisque varius morbi enim nunc faucibus.
In egestas erat imperdiet sed euismod nisi. Cursus metus aliquam eleifend mi in nulla posuere sollicitudin aliquam.
Parturient montes nascetur ridiculus mus mauris vitae ultricies. Eros donec ac odio tempor orci.
Enim facilisis gravida neque convallis a cras semper auctor. Odio pellentesque diam volutpat commodo.
Volutpat commodo sed egestas egestas fringilla phasellus.
Neque egestas congue quisque egestas diam in arcu cursus euismod.
Adipiscing diam donec adipiscing tristique risus. Quis auctor elit sed vulputate mi sit amet mauris commodo.
Quis blandit turpis cursus in hac habitasse platea dictumst. Praesent semper feugiat nibh sed pulvinar.
Lorem mollis aliquam ut porttitor leo a. Sollicitudin nibh sit amet commodo nulla facilisi nullam vehicula ipsum.
Tortor dignissim convallis aenean et tortor at risus viverra adipiscing. Ac turpis egestas sed tempus urna et.
Lectus proin nibh nisl condimentum id venenatis a condimentum vitae. Tempus egestas sed sed risus pretium quam.
Tortor dignissim convallis aenean et tortor. Aliquet nec ullamcorper sit amet risus.
Faucibus in ornare quam viverra orci sagittis eu volutpat odio. Quis eleifend quam adipiscing vitae proin sagittis nisl.
Vestibulum rhoncus est pellentesque elit ullamcorper dignissim cras.
Morbi tincidunt augue interdum velit euismod in pellentesque massa placerat.
Pellentesque massa placerat duis ultricies lacus sed turpis tincidunt.
Sit amet nisl purus in mollis. Posuere sollicitudin aliquam ultrices sagittis orci a.";
#endif


/*! \} */
