/*******************************************************************************
 * Copyright 2016 -- 2022 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*******************************************************************************/

/*****************************************************************************
 * @file       warp_transform.hpp
 * @brief      The Role for a WarpTransform Example application (UDP or TCP)
 * @author     DCO
 * @date       Nov 2021
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
 * @ingroup WarpTransformHLS
 * @addtogroup WarpTransformHLS
 * \{
 *****************************************************************************/


#ifndef _ROLE_WARPTRANSFORM_H_
#define _ROLE_WARPTRANSFORM_H_

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>
#include "network.hpp"
#include "memory_utils.hpp"

using namespace hls;

// Define this option to load data from network to DDR memory before calling the kernel.
#define ENABLE_DDR
//#define FAKE_WarpTransform
/********************************************
 * SHELL/MMIO/EchoCtrl - Config Register
 ********************************************/
enum EchoCtrl {
	ECHO_PATH_THRU	= 0,
	ECHO_STORE_FWD	= 1,
	ECHO_OFF	= 2
};


/********************************************
 * Internal WarpTransform accelerator command
 ********************************************/
enum MemTestCmd {
    WRPTX_IMG_CMD = 2,
    WRPTX_TXMAT_CMD  = 1,
    WRPTX_INVLD_CMD = 0
};

//CMD 8 bitwdith up to 255 commands (0 is invalid)
#define WARPTRANSFORM_COMMANDS_HIGH_BIT WARPTRANSFORM_COMMANDS_BITWIDTH-1
#define WARPTRANSFORM_COMMANDS_LOW_BIT 0
#define WARPTRANSFORM_COMMANDS_BITWIDTH 8
typedef unsigned int  img_meta_t; 
#define TRANSFORM_MATRIX_DIM 9
const unsigned int const_tx_matrix_dim=TRANSFORM_MATRIX_DIM;
#define ROLE_IS_WARPTRANSFORM

#define WAIT_FOR_META             0
#define WAIT_FOR_STREAM_PAIR      1
#define PROCESSING_PACKET         2
#define LOAD_IN_STREAM            3
#define WARPTRANSFORM_RETURN_RESULTS     4
#define WARPTRANSFORM_RETURN_RESULTS_ABSORB_DDR_LAT 5
#define WARPTRANSFORM_RETURN_RESULTS_UNPACK 6
#define WARPTRANSFORM_RETURN_RESULTS_FWD 7
#define WAIT_FOR_TX               8
#define FSM_IDLE                    9
#define FSM_CHK_SKIP                10
#define FSM_CHK_PROC_BYTES          11
#define FSM_CHK_WRT_CHNK_TO_DDR_PND 12
#define FSM_WR_PAT_CMD              13
#define FSM_WR_PAT_LOAD             14
#define FSM_WR_PAT_DATA             15
#define FSM_WR_PAT_STS_A            16
#define FSM_WR_PAT_STS_B            17
#define FSM_WR_PAT_STS_C            18
#define PROCESSING_PACKET_TXMAT     19           
#define PROCESSING_PACKET_IMGMAT    20           
#define WAIT_FOR_META_IMGMAT        21           
#define PUSH_REMAINING_META         22
#define WAIT_FOR_IMAGE_DIMENSIONS   23           

#define PacketFsmType uint8_t


#define DEFAULT_TX_PORT 2718
#define DEFAULT_RX_PORT 2718
// Starting with 2718, this number corresponds to the extra opened ports of this role. Every bit set
// corresponds to one port.
// e.g. 0x1->2718, 0x2->2719, 0x3->[2718,2719], 0x7->[2718,2719,2720], 0x17->[2718-2722], etc.
#define PORTS_OPENED 0x1F

#define Data_t_in  ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0>
#define Data_t_out ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0>


#define MAX_NB_OF_ELMT_READ  16
typedef uint8_t  mat_elmt_t;    // change to float or double depending on your needs

#define MAX_NB_OF_WORDS_READ	(MAX_NB_OF_ELMT_READ*sizeof(mat_elmt_t)/BPERDW) // =2 if double =1 if float
#define MAX_NB_OF_ELMT_PERDW	(BPERDW/sizeof(mat_elmt_t)) // =8 if double =16 if float


//------------------------------------ Declarations for DDR ----------------------------------------

/* General memory Data Width is set as a parameter*/
/* 52-bit host AXI data width*/
#define MEMDW_512 512               // 512 Bus width in bits for cF DDR memory
#define BPERMDW_512 (MEMDW_512/8)   // Bytes per DDR Memory Data Word,  if MEMDW=512 => BPERMDW_512 = 64
#define KWPERMDW_512 (BPERMDW_512/sizeof(TYPE)) // Number of WarpTransform kernel words per DDR memory word
typedef ap_uint<MEMDW_512>  membus_512_t;   /* 512-bit ddr memory access */
typedef membus_512_t membus_t;
#define TOTMEMDW_512 (1 + (IMGSIZE - 1) / BPERMDW_512)

/***************************************************************************************************
 * @brief This define configures tha AXI burst size of DDRM memory-mapped interfaces 
 * AXI4 allows 4KiB, but Role's AXI interconnect is configured at max 1KiB
 * 0x40->64, 0x400->1024B(1KiB), 0x1000->4KiB
 **************************************************************************************************/
#define CHECK_CHUNK_SIZE 0x1000 
#define BYTE_PER_MEM_WORD BPERMDW_512 // 64
#define TRANSFERS_PER_CHUNK (CHECK_CHUNK_SIZE/BYTE_PER_MEM_WORD) //64
#define TRANSFERS_PER_CHUNK_DIVEND (TOTMEMDW_512-(TOTMEMDW_512/TRANSFERS_PER_CHUNK)*TRANSFERS_PER_CHUNK)


#define fsmStateDDRdef uint8_t

// The maximum number of cycles allowed to acknowledge a write to DDR (i.e. read the status stream)
#define CYCLES_UNTIL_TIMEOUT 0x0100
#define TYPICAL_DDR_LATENCY 4
// The latency cycles of cF DDR. We've measured 52, but experimentally we take it if we divide by 
// 4.769230769, taking into account the II=2 and the latency of the FSM
#define DDR_LATENCY (52/4)
#define EXTRA_DDR_LATENCY_DUE_II (64 + 8) // 8 is the write from input stream to local stream, 64 is read from local stream to DDR
/*
 * A generic unsigned AXI4-Stream interface used all over the cloudFPGA place.
 */
//Consider using axi_utils.hpp header? 
// though some difference in the init
template<int D>
struct Axis {
  ap_uint<D>       tdata;
  ap_uint<(D+7)/8> tkeep;
  ap_uint<1>       tlast;
  Axis() {}
  Axis(ap_uint<D> single_data) : tdata((ap_uint<D>)single_data), tkeep(1), tlast(1) {}
};

void warp_transform(

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
    //---- Read Path (MM2S) ------------
    // stream<DmCmd>               &soMemRdCmdP0,
    // stream<DmSts>               &siMemRdStsP0,
    // stream<Axis<MEMDW_512 > >   &siMemReadP0,
    //---- Write Path (S2MM) -----------
    stream<DmCmd>               &soMemWrCmdP0,
    stream<DmSts>               &siMemWrStsP0,
    stream<Axis<MEMDW_512> >    &soMemWriteP0,
    //------------------------------------------------------
    //-- SHELL / Role / Mem / Mp1 Interface
    //------------------------------------------------------             
    membus_t   *lcl_mem0,
    membus_t   *lcl_mem1
    #endif
);


#endif


/*! \} */
