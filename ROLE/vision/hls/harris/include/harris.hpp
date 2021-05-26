/*****************************************************************************
 * @file       harris.hpp
 * @brief      The Role for a Harris Example application (UDP or TCP)
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
 * @ingroup HarrisHLS
 * @addtogroup HarrisHLS
 * \{
 *****************************************************************************/


#ifndef _ROLE_HARRIS_H_
#define _ROLE_HARRIS_H_

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>

#include "network.hpp"

using namespace hls;

// Define this option to load data from network to DDR memory before calling the kernel.
#define ENABLE_DDR

/********************************************
 * SHELL/MMIO/EchoCtrl - Config Register
 ********************************************/
enum EchoCtrl {
	ECHO_PATH_THRU	= 0,
	ECHO_STORE_FWD	= 1,
	ECHO_OFF	= 2
};




#define WAIT_FOR_META             0
#define WAIT_FOR_STREAM_PAIR      1
#define PROCESSING_PACKET         2
#define HARRIS_RETURN_RESULTS     3
#define HARRIS_RETURN_RESULTS_FWD 4

#define PacketFsmType uint8_t


#define DEFAULT_TX_PORT 2718
#define DEFAULT_RX_PORT 2718

#define Data_t_in  ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0>
#define Data_t_out ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0>

/* General memory Data Width is set as a parameter*/
/* 52-bit host AXI data width*/
#define MEMDW_512 512               // 512 Bus width in bits for cF DDR memory
#define BPERMDW_512 (MEMDW_512/8)   // Bytes per DDR Memory Data Word,  if MEMDW=512 => BPERMDW_512 = 64
#define KWPERMDW_512 (BPERMDW_512/sizeof(IN_TYPE)) // Number of Harris kernel words per DDR memory word
typedef ap_uint<MEMDW_512>  membus_512_t;   /* 512-bit ddr memory access */
typedef membus_512_t membus_t;
#define TOTMEMDW_512 (1 + (IMGSIZE - 1) / BPERMDW_512)

#define MAX_NB_OF_ELMT_READ  16
typedef uint8_t  mat_elmt_t; 	// change to float or double depending on your needs

#define MAX_NB_OF_WORDS_READ	(MAX_NB_OF_ELMT_READ*sizeof(mat_elmt_t)/BPERDW) // =2 if double =1 if float
#define MAX_NB_OF_ELMT_PERDW	(BPERDW/sizeof(mat_elmt_t)) // =8 if double =16 if float


void harris(

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
    
    #ifdef ENABLE_DDR
                                            ,
    //------------------------------------------------------
    //-- SHELL / Role / Mem / Mp0 Interface
    //------------------------------------------------------

    membus_t   *lcl_mem0,
    membus_t   *lcl_mem1
    #endif
);


#endif


/*! \} */
