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


/* Instead of the actual MCE IP, use sipmle logic that assigns values on out. For fast HLS cim/cosim
 * uncomment this, else keep it commented.
 */
#define FAKE_MCEuropeanEngine

#define DEFAULT_TX_PORT 2718
#define DEFAULT_RX_PORT 2718

//#define USE_HLSLIB_DATAFLOW

//#define USE_HLSLIB_STREAM

#if DtUsed == double
#define INPUT_PTR_WIDTH 64
#elif DtUsed == float
#define INPUT_PTR_WIDTH 32
#endif
#define OUTPUT_PTR_WIDTH 64


#define INSIZE sizeof(varin)
#define OUTSIZE sizeof(DtUsed)*OUTDEP

#define BITS_PER_10GBITETHRNET_AXI_PACKET 64
#define BYTES_PER_10GBITETHRNET_AXI_PACKET (BITS_PER_10GBITETHRNET_AXI_PACKET/8)

#define IN_PACKETS INSIZE/(BYTES_PER_10GBITETHRNET_AXI_PACKET)
#define OUT_PACKETS OUTSIZE/(BYTES_PER_10GBITETHRNET_AXI_PACKET)

#define MIN_RX_LOOPS IN_PACKETS*(BITS_PER_10GBITETHRNET_AXI_PACKET/INPUT_PTR_WIDTH)
#define MIN_TX_LOOPS OUT_PACKETS*(BITS_PER_10GBITETHRNET_AXI_PACKET/OUTPUT_PTR_WIDTH)

#define WAIT_FOR_META        0
#define WAIT_FOR_STREAM_PAIR 1
#define PROCESSING_PACKET    2
#define PROCESSING_WAIT      3
#define MCEUROPEANENGINE_RETURN_RESULTS 4

#define PacketFsmType uint8_t


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