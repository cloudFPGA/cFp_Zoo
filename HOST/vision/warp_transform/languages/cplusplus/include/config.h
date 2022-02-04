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
 * @file       config.h
 * @brief      The configuration of a WarpTransform Example application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID, DCO
 * @date       June 2020
 *----------------------------------------------------------------------------
 *
 * @details    This file contains definitios used both in HLS and host code of 
 *             cFp Vitis WarpTransform application example.
 *
 *----------------------------------------------------------------------------
 * 
 * @ingroup WarpTransform
 * @addtogroup WarpTransform
 * \{
 *****************************************************************************/

//------------------------------------  USEFUL MACROS --------------------------------------------

/** Ceiling function without using math.h                                                         */
#define CEIL(a, b)     (((a) + (b-1)) / (b)) 



//--------------------------------  USER DEFINED OPTIONS ------------------------------------------
/** The maximum width of frame in pixels   from 6x6 to 256x256 for debugging                      */
#define FRAME_HEIGHT 1024 

/** The maximum height of frame in pixels                                                         */
#define FRAME_WIDTH  1024

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
#define TB_SIM_CFP_VITIS

/** Keep it uncommented of you want the input to be from camera frames else, for images comment it*/
//  #define INPUT_FROM_CAMERA

/** For The OpenCV type fot th input image. TODO: We have to automatically fix it for every kernel*/
#define INPUT_TYPE_HOST CV_8UC1

/** The network socket type: tcp or udp                                                           */
#define NET_TYPE udp

/** The level of debugging.
 *  0->None, 1-> Light Debug, 2-> Medium Debug , 3-> Insane Debug
 */
#define DEBUG_LEVEL dbgLevelNone

//-------------------  AUTOMATICALLY DEFINED OR AUXILILIARY OPTIONS  -------------------------------

#define FRAME_TOTAL FRAME_HEIGHT * FRAME_WIDTH //* 3
#define CMD_OVERHEAD_BYTES (2 + 5) * 8 // 8 bytes x 2 commands, and the tx matrix
#define WARP_TRANSFORM_TOTAL FRAME_TOTAL +  CMD_OVERHEAD_BYTES

/** The total TxRx transfers for a predefined MTU=PACK_SIZE                                       */
#define TOT_TRANSFERS_TX CEIL(WARP_TRANSFORM_TOTAL, PACK_SIZE)
#define TOT_TRANSFERS_RX CEIL(FRAME_TOTAL, PACK_SIZE)

#define tcp 0
#define udp 1

#define PY_WRAP_WARPTRANSFORM_NUMPI 0
#define PY_WRAP_WARPTRANSFORM_FILENAME 1

/*! \} */
