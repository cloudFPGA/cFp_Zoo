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

/** Ceiling function without using math.h                                                         */
#define CEIL(a, b)     (((a) + (b-1)) / (b)) 



//--------------------------------  USER DEFINED OPTIONS ------------------------------------------
/** The maximum width of frame in pixels                                                          */
#define FRAME_HEIGHT 512

/** The maximum height of frame in pixels                                                         */
#define FRAME_WIDTH  512

#define FRAME_INTERVAL (1000/30)

/** This is our custom MTU. We must use a multiple of 8 (Bytes per transaction)! 1450 4086 udp pack 
 * size; note that OSX limits < 8100 bytes                                                        */
#define PACK_SIZE 1024

/** Larger than maximum UDP packet size                                                           */
#define BUF_LEN 65540

/** If defined, output images will be written                                                     */
#define WRITE_OUTPUT_FILE  

/** If defined, images will be shown in pop-up windows                                            */
//  #define SHOW_WINDOWS  
  
/** For HOST TB uncomment this. For normal host execution keep it commented                       */
//  #define TB_SIM_CFP_VITIS

/** Keep it uncommented of you want the input to be from camera frames else, for images comment it*/
//  #define INPUT_FROM_CAMERA

/** For The OpenCV type fot th input image. TODO: We have to automatically fix it for every kernel*/
#define INPUT_TYPE_HOST CV_8UC1

/** The network socket type: tcp or udp                                                           */
#define NET_TYPE udp

//-------------------  AUTOMATICALLY DEFINED OR AUXILILIARY OPTIONS  -------------------------------

#define FRAME_TOTAL FRAME_HEIGHT * FRAME_WIDTH //* 3

/** The total TxRx transfers for a predefined MTU=PACK_SIZE                                       */
#define TOT_TRANSFERS CEIL(FRAME_TOTAL, PACK_SIZE)  

#define tcp 0
#define udp 1

#define PY_WRAP_HARRIS_NUMPI 0
#define PY_WRAP_HARRIS_FILENAME 1

/*! \} */
