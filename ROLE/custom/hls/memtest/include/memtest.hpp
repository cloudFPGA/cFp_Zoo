/*****************************************************************************
 * @file       memtest.hpp
 * @brief      The Role for a Memtest Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID, DCO
 * @date       September 2021
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
#include <bitset>

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

/********************************************
 * Internal MemTest accelerator command
 ********************************************/
enum MemTestCmd {
    TEST_ENDOFTESTS_CMD  = 3,
    TEST_STOP_CMD  = 2,
    TEST_START_CMD = 1,
    TEST_INVLD_CMD = 0
};
//CMD 8 bitwdith up to 255 commands (0 is invalid)
#define MEMTEST_COMMANDS_HIGH_BIT 8-1
#define MEMTEST_COMMANDS_LOW_BIT 0

#define WAIT_FOR_META 0
#define WAIT_FOR_STREAM_PAIR 1
#define PROCESSING_PACKET 2
#define MEMTEST_RETURN_RESULTS 3

#define PacketFsmType uint8_t

#define FSM_WRITE_NEW_DATA 0
#define FSM_DONE 1
#define PortFsmType uint8_t

#define DEFAULT_TX_PORT 2718
#define DEFAULT_RX_PORT 2718


//------------------------------------ Declarations for DDR ----------------------------------------

/* General memory Data Width is set as a parameter*/
/* 512-bit host AXI data width*/
#define MEMDW_512 512               // 512 Bus width in bits for cF DDR memory
#define BPERMDW_512 (MEMDW_512/8)   // Bytes per DDR Memory Data Word,  if MEMDW=512 => BPERMDW_512 = 64
#define KWPERMDW_512 (BPERMDW_512/sizeof(IN_TYPE)) // Number of Harris kernel words per DDR memory word
typedef ap_uint<MEMDW_512>  membus_512_t;   /* 512-bit ddr memory access */
typedef membus_512_t membus_t;
#define TOTMEMDW_512 1200

#define CHECK_CHUNK_SIZE 0x40 // 0x40 -> 64, 0x1000 -> 4 KiB
#define BYTE_PER_MEM_WORD BPERMDW_512 // 64
#define TRANSFERS_PER_CHUNK (CHECK_CHUNK_SIZE/BYTE_PER_MEM_WORD) //64

//typedef enum fsmStateDDRenum {
//    FSM_WR_PAT_CMD	= 0,
//    FSM_WR_PAT_DATA	= 1,
//    FSM_WR_PAT_STS  = 2
//} fsmStateDDRdef;
//typedef enum fsmStateDDRenum fsmStateDDRdef;

#define fsmStateDDRdef uint8_t

// The maximum number of cycles allowed to acknowledge a write to DDR (i.e. read the status stream)
#define CYCLES_UNTIL_TIMEOUT 0x0100
#define TYPICAL_DDR_LATENCY 4
#define DDR_LATENCY 52 // The latency cycles of cF DDR
#define EXTRA_DDR_LATENCY_DUE_II (64 + 8) // 8 is the write from input stream to local stream, 64 is read from local stream to DDR



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
        #ifdef ENABLE_DDR
                                            ,
    //------------------------------------------------------
    //-- SHELL / Role / Mem / Mp1 Interface
    //------------------------------------------------------             
    membus_t   *lcl_mem0,
    membus_t   *lcl_mem1
    #endif
);


#endif

/*! \} */
