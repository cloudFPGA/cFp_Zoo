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




/********************************************
 * SHELL/MMIO/EchoCtrl - Config Register
 ********************************************/
enum EchoCtrl {
	ECHO_PATH_THRU	= 0,
	ECHO_STORE_FWD	= 1,
	ECHO_OFF		= 2
};

/********************************************
 * UDP Specific Streaming Interfaces
 ********************************************/

//struct UdpWord {            // UDP Streaming Chunk (i.e. 8 bytes)
//    ap_uint<64>    tdata;
//    ap_uint<8>     tkeep;
//    ap_uint<1>     tlast;
//    UdpWord()      {}
//    UdpWord(ap_uint<64> tdata, ap_uint<8> tkeep, ap_uint<1> tlast) :
//                   tdata(tdata), tkeep(tkeep), tlast(tlast) {}
//};


void harris_app (

        //------------------------------------------------------
        //-- SHELL / This / Mmio / Config Interfaces
        //------------------------------------------------------
        ap_uint<2>          piSHL_This_MmioEchoCtrl,
        //[TODO] ap_uint<1> piSHL_This_MmioPostPktEn,
        //[TODO] ap_uint<1> piSHL_This_MmioCaptPktEn,

        //------------------------------------------------------
        //-- SHELL / This / Udp Interfaces
        //------------------------------------------------------
        stream<UdpWord>     &siSHL_This_Data,
        stream<UdpWord>     &soTHIS_Shl_Data
);


#endif
