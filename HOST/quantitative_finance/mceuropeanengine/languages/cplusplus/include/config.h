/*****************************************************************************
 * @file       config.h
 * @brief      The configuration of a Uppercase Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID
 * @date       June 2020
 *----------------------------------------------------------------------------
 *
 * @details    This file contains definitios used both in HLS and host code of 
 *             cFp Vitis Uppercase application example.
 *
 *----------------------------------------------------------------------------
 * 
 * @ingroup Uppercase
 * @addtogroup Uppercase
 * \{
 *****************************************************************************/

//------------------------------------  USEFUL MACROS --------------------------------------------

/** Ceiling function without using math.h                                                         */
#define CEIL(a, b)     (((a) + (b-1)) / (b)) 



//--------------------------------  USER DEFINED OPTIONS ------------------------------------------

/** This is our custom MTU. We must use a multiple of 8 (Bytes per transaction)! 1450 4086 udp pack 
 * size; note that OSX limits < 8100 bytes                                                        */
#define PACK_SIZE 1024

/** Larger than maximum UDP packet size                                                           */
#define BUF_LEN 65540              

/*  For HOST TB uncomment this. For normal host execution keep it commented                       */
//  #define TB_SIM_CFP_VITIS
  
/** The network socket type: tcp or udp                                                           */
#define NET_TYPE udp

//----------------------------  AUTOMATICALLY DEFINED OPTIONS  -------------------------------------


#define FRAME_TOTAL INSIZE + OUTSIZE

/** The total TxRx transfers for a predefined MTU=PACK_SIZE                                       */
#define TOT_TRANSFERS CEIL(FRAME_TOTAL, PACK_SIZE)  

#define tcp 0
#define udp 1

/*! \} */
