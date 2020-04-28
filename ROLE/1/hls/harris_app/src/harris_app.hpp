//  *
//  *                       cloudFPGA
//  *     Copyright IBM Research, All Rights Reserved
//  *    =============================================
//  *     Created: April 2020
//  *     Authors: FAB, WEI, NGL, DID
//  *
//  *     Description:
//  *        The Role for a Harris Example application (UDP or TCP)
//  *

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

//#include "../../../../../cFDK/SRA/LIB/hls/network.hpp"
#include "network.hpp"

using namespace hls;


#define WAIT_FOR_META 0
#define WAIT_FOR_STREAM_PAIR 1
#define PROCESSING_PACKET 2
#define PacketFsmType uint8_t


#define DEFAULT_TX_PORT 2718
#define DEFAULT_RX_PORT 2718


void harris_app(

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
