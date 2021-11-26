/*****************************************************************************
 * @file       config.h
 * @brief      The configuration of a Memtest Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID, DCO
 * @date       June 2020
 *----------------------------------------------------------------------------
 *
 * @details    This file contains definitios used both in HLS and host code of 
 *             cFp Vitis Memtest application example.
 *
 *----------------------------------------------------------------------------
 * 
 * @ingroup Memtest
 * @addtogroup Memtest
 * \{
 *****************************************************************************/

//------------------------------------  USEFUL MACROS --------------------------------------------

/** Ceiling function without using math.h                                                         */
#define CEIL(a, b)     (((a) + (b-1)) / (b)) 



//--------------------------------  USER DEFINED OPTIONS ------------------------------------------
#define TRACE_OFF     0x0000
#define TRACE_URIF   1 <<  1
#define TRACE_UAF    1 <<  2
#define TRACE_MMIO   1 <<  3
#define TRACE_ALL     0xFFFF
#define DEBUG_LEVEL (TRACE_ALL)

/** This is our custom MTU. We must use a multiple of 8 (Bytes per transaction)! 1450 4086 udp pack 
 * size; note that OSX limits < 8100 bytes                                                        */
#define PACK_SIZE 1024

/** Larger than maximum UDP packet size                                                           */
#define BUF_LEN 65540              

/*  For HOST TB uncomment this. For normal host execution keep it commented                       */
  #define TB_SIM_CFP_VITIS
  
/** The network socket type: tcp or udp                                                           */
#define NET_TYPE udp

//----------------------------  AUTOMATICALLY DEFINED OPTIONS  -------------------------------------


#define tcp 0
#define udp 1

/*! \} */
