/*****************************************************************************
 * @file       config.h
 * @brief      The configuration of a Harris Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID
 * @date       June 2020
 *----------------------------------------------------------------------------
 *
 * @details    This file contains definitios used both in HLS and host code of 
 *             cFp Vitis Harris application example.
 *
 *----------------------------------------------------------------------------
 * 
 * @ingroup Harris
 * @addtogroup Harris
 * \{
 *****************************************************************************/

//------------------------------------  USEFUL MACROS --------------------------------------------
#define CEIL(a, b)     (((a) + (b-1)) / (b)) /** Ceiling function without using math.h            */



//--------------------------------  USER DEFINED OPTIONS ------------------------------------------
/** The maximum width of frame in pixels                                                          */
#define FRAME_HEIGHT 256           

/** The maximum height of frame in pixels                                                         */
#define FRAME_WIDTH  256        

#define FRAME_INTERVAL (1000/30)

/** This is our custom MTU. We must use a multiple of 8 (Bytes per transaction)! 1450 4086 udp pack 
 * size; note that OSX limits < 8100 bytes                                                        */
#define PACK_SIZE 1024          

/** Larger than maximum UDP packet size                                                           */
#define BUF_LEN 65540              

/** If defined, output images will be written                                                     */
//  #define WRITE_OUTPUT_FILE  

/*  For HOST TB uncomment this. For normal host execution keep it commented                       */
//  #define TB_SIM_CFP_VITIS           

                                       

//----------------------------  AUTOMATICALLY DEFINED OPTIONS  -------------------------------------
#define TOT_TRANSFERS CEIL(FRAME_HEIGHT*FRAME_WIDTH, PACK_SIZE)  /** The total TxRx transfers for 
                                                                     a predefined MTU=PACK_SIZE   */



/*! \} */