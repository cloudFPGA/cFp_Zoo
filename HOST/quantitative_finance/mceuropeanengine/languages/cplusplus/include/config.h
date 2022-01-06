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
 * @brief      The configuration of a MCEuropeanEngine application (UDP or TCP)
 * @author     FAB, WEI, NGL, DID
 * @date       September 2020
 *----------------------------------------------------------------------------
 *
 * @details    This file contains definitios used both in HLS and host code of 
 *             cFp Vitis MCEuropeanEngine application.
 *
 *----------------------------------------------------------------------------
 * 
 * @ingroup MCEuropeanEngine
 * @addtogroup MCEuropeanEngine
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
//#define TB_SIM_CFP_VITIS
  
/** The network socket type: tcp or udp                                                           */
#define NET_TYPE udp

/** The data type of MCEuropeanEngine, that determines the precision. Either float or double.     */
#define DtUsed double

/** The number of MCEuropean Engines running in parallel with differnet seed.                     */
#define MCM_NM 6

/** The number of execution loops, thus the depth of the output values' vector.                   */
#define OUTDEP 1024

//----------------------------  AUTOMATICALLY DEFINED OPTIONS  -------------------------------------

/** The total TxRx transfers for a predefined MTU=PACK_SIZE                                       */
#define TOT_TRANSFERS_IN (CEIL(INSIZE, PACK_SIZE))
#define TOT_TRANSFERS_OUT (CEIL(OUTSIZE, PACK_SIZE))
#define TOT_TRANSFERS TOT_TRANSFERS_IN + TOT_TRANSFERS_OUT

#if DtUsed == double
//#define DtUsedInt long unsigned int
typedef long unsigned int DtUsedInt;
#elif DtUsed == float
#define DtUsedInt unsigned int
#endif

#define tcp 0
#define udp 1

//---------------------------------  USER DEFINED STRUCTS ------------------------------------------

/** The struct holding the input parameters of MCEuropeanEngine                                   */
struct varin {
  DtUsedInt loop_nm;
  DtUsedInt seed;
  DtUsed    underlying;
  DtUsed    volatility;
  DtUsed    dividendYield;
  DtUsed    riskFreeRate;
  DtUsed    timeLength;
  DtUsed    strike;
  DtUsedInt optionType;
  DtUsed    requiredTolerance;
  DtUsedInt requiredSamples;
  DtUsedInt timeSteps;
  DtUsedInt maxSamples;
};

/** A union to convert between DtUsed and DtUsedInt                                               */
union intToFloatUnion{
    DtUsed f;
    DtUsedInt i;
};


/*! \} */
