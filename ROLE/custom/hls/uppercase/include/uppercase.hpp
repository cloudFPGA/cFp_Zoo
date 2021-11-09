/*****************************************************************************
 * @file       uppercase.hpp
 * @brief      The Role for a Uppercase Example application (UDP or TCP)
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
 * @ingroup UppercaseHLS
 * @addtogroup UppercaseHLS
 * \{
 *****************************************************************************/


#ifndef _ROLE_UPPERCASE_H_
#define _ROLE_UPPERCASE_H_

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


#define ENABLE_DDR 
#define ROLE_IS_UPPERCASE
/********************************************
 * SHELL/MMIO/EchoCtrl - Config Register
 ********************************************/
enum EchoCtrl {
	ECHO_PATH_THRU	= 0,
	ECHO_STORE_FWD	= 1,
	ECHO_OFF	= 2
};


#define WAIT_FOR_META 0
#define WAIT_FOR_STREAM_PAIR 1
#define PROCESSING_PACKET 2
#define UPPERCASE_RETURN_RESULTS 3

#define PacketFsmType uint8_t

#define FSM_WRITE_NEW_DATA 0
#define FSM_DONE 1
#define PortFsmType uint8_t

#define DEFAULT_TX_PORT 2718
#define DEFAULT_RX_PORT 2718


//#define MEMDW 64          // 512 or 128 or 64 // Bus width in bits for Host memory
//#define BPERDW (MEMDW/8)   // Bytes per Data Word    if MEMDW=512 => BPERDW = 64, if MEMDW=64 => BPERDW = 16

//------------------------------------ Declarations for DDR ----------------------------------------

/* General memory Data Width is set as a parameter*/
/* 512-bit host AXI data width*/
#define MEMDW_512 512               // 512 Bus width in bits for cF DDR memory
typedef ap_uint<MEMDW_512>  membus_512_t;   /* 512-bit ddr memory access */
typedef membus_512_t membus_t;
#define TOTMEMDW_512 100 //512b * 100 = 640B

// The maximum number of cycles allowed to acknowledge a write to DDR (i.e. read the status stream)
#define CYCLES_UNTIL_TIMEOUT 0x0100
#define TYPICAL_DDR_LATENCY 4
#define DDR_LATENCY 52 // The latency cycles of cF DDR
#define EXTRA_DDR_LATENCY_DUE_II (64 + 8) // 8 is the write from input stream to local stream, 64 is read from local stream to DDR


void uppercase(

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
    //-- SHELL / Role / Mem / Mp1 Interface
    //------------------------------------------------------             
    membus_t   *lcl_mem0,
    membus_t   *lcl_mem1
    #endif//ENABLE_DDR
);


#endif//_ROLE_UPPERCASE_H_


/*! \} */