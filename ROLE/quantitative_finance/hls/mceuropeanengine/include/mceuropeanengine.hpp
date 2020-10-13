/*****************************************************************************
 * @file       mceuropeanengine.hpp
 * @brief      The Role for a MCEuropeanEngine Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID
 * @date       October 2020
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
 * @ingroup MCEuropeanEngineHLS
 * @addtogroup MCEuropeanEngineHLS
 * \{
 *****************************************************************************/


#ifndef _ROLE_MCEUROPEANENGINE_H_
#define _ROLE_MCEUROPEANENGINE_H_

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

#include "kernel_mceuropeanengine.hpp"

using namespace hls;


/********************************************
 * SHELL/MMIO/EchoCtrl - Config Register
 ********************************************/
enum EchoCtrl {
	ECHO_PATH_THRU	= 0,
	ECHO_STORE_FWD	= 1,
	ECHO_OFF	= 2
};


#define INPUT_PTR_WIDTH 64
#define OUTPUT_PTR_WIDTH 64


struct varin {
  unsigned int loop_nm;
  unsigned int seed;
  DtUsed underlying;
  DtUsed volatility;
  DtUsed dividendYield;
  DtUsed riskFreeRate;
  DtUsed timeLength;
  DtUsed strike;
  unsigned int optionType; // bool
  DtUsed requiredTolerance;
  unsigned int requiredSamples;
  unsigned int timeSteps;
  unsigned int maxSamples;
};


#define INSIZE sizeof(varin)
#define OUTSIZE sizeof(DtUsed)*OUTDEP

#define BITS_PER_10GBITETHRNET_AXI_PACKET 64
#define BYTES_PER_10GBITETHRNET_AXI_PACKET (BITS_PER_10GBITETHRNET_AXI_PACKET/8)

#define IN_PACKETS INSIZE/(BYTES_PER_10GBITETHRNET_AXI_PACKET)
#define OUT_PACKETS OUTSIZE/(BYTES_PER_10GBITETHRNET_AXI_PACKET)

#define MIN_RX_LOOPS IN_PACKETS*(BITS_PER_10GBITETHRNET_AXI_PACKET/INPUT_PTR_WIDTH)
#define MIN_TX_LOOPS OUT_PACKETS*(BITS_PER_10GBITETHRNET_AXI_PACKET/OUTPUT_PTR_WIDTH)

#define WAIT_FOR_META 0
#define WAIT_FOR_STREAM_PAIR 1
#define PROCESSING_PACKET 2
#define MCEUROPEANENGINE_RETURN_RESULTS 3

#define PacketFsmType uint8_t

#define DEFAULT_TX_PORT 2718
#define DEFAULT_RX_PORT 2718


#define MEMDW 64          // 512 or 128 or 64 // Bus width in bits for Host memory
#define BPERDW (MEMDW/8)   // Bytes per Data Word    if MEMDW=512 => BPERDW = 64, if MEMDW=64 => BPERDW = 16

#define MAX_NB_OF_ELMT_READ  16
typedef uint8_t  mat_elmt_t; 	// change to float or double depending on your needs

#define MAX_NB_OF_WORDS_READ	(MAX_NB_OF_ELMT_READ*sizeof(mat_elmt_t)/BPERDW) // =2 if double =1 if float
#define MAX_NB_OF_ELMT_PERDW	(BPERDW/sizeof(mat_elmt_t)) // =8 if double =16 if float


void mceuropeanengine(

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


#endif


/*! \} */