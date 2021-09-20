/*****************************************************************************
 * @file       commom.hpp
 * @brief      Common functions for testbenches - headers.
 *
 * @date       June 2020
 * @author     FAB, WEI, NGL, DID
 * 
 * Copyright 2009-2015 - Xilinx Inc.  - All rights reserved.
 * Copyright 2015-2020 - IBM Research - All Rights Reserved.
 *
 * @ingroup VitisVision 
 * @addtogroup VitisVision 
 * \{
 *****************************************************************************/


#ifndef _TEST_COMMON_H_
#define _TEST_COMMON_H_


#include <stdio.h>
#include <string>
#include <hls_stream.h>
#include <sys/stat.h>
//#include "../../uppercase/include/uppercase.hpp"
#include "../../memtest/include/memtest.hpp"
#include "../../../../../HOST/custom/uppercase/languages/cplusplus/include/config.h"
#include <bits/stdc++.h>

//---------------------------------------------------------
// HELPERS FOR THE DEBUGGING TRACES
//  .e.g: DEBUG_LEVEL = (MDL_TRACE | IPS_TRACE)
//---------------------------------------------------------
#define THIS_NAME "TB"

#define TRACE_OFF     0x0000
#define TRACE_URIF   1 <<  1
#define TRACE_UAF    1 <<  2
#define TRACE_MMIO   1 <<  3
#define TRACE_ALL     0xFFFF

#define DEBUG_LEVEL (TRACE_ALL)


//------------------------------------------------------
//-- TESTBENCH DEFINES
//------------------------------------------------------
#define OK          true
#define KO          false
#define VALID       true
#define UNVALID     false
#define DEBUG_TRACE true

#define ENABLED     (ap_uint<1>)1
#define DISABLED    (ap_uint<1>)0




/*****************************************************************************
 * @brief Initialize an input data stream from a file.
 *
 * @param[in] sDataStream the input data stream to set.
 * @param[in] dataStreamName the name of the data stream.
 * @param[in] inpFileName the name of the input file to read from.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool setInputDataStream(stream<UdpWord> &sDataStream, const std::string dataStreamName, 
			const std::string inpFileName, int simCnt);


/*****************************************************************************
 * @brief Read data from a stream.
 *
 * @param[in]  sDataStream,    the output data stream to read.
 * @param[in]  dataStreamName, the name of the data stream.
 * @param[out] udpWord,        a pointer to the storage location of the data
 *                              to read.
 * @return VALID if a data was read, otherwise UNVALID.
 ******************************************************************************/
bool readDataStream(stream <UdpWord> &sDataStream, UdpWord *udpWord);


/*****************************************************************************
 * @brief Pack an array of 8 x ap_uint<8> into a ap_uint<64> word.
 *
 * @param[in]  buffer     A pointer to an array of 8 x ap_uint<8>
 * @return An ap_uint<64> word.
 ******************************************************************************/
ap_uint<64> pack_ap_uint_64_ (ap_uint<8> *buffer);


/*****************************************************************************
 * @brief Dump a data word to a file.
 *
 * @param[in] udpWord,      a pointer to the data word to dump.
 * @param[in] outFileStream,the output file stream to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool dumpDataToFile(UdpWord *udpWord, std::ofstream &outFileStream);


/*****************************************************************************
 * @brief Initialize an input data stream from a file.
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] strOutput the output string to set.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool dumpFileToString(const std::string inpFileName, std::string strOutput, int simCnt);


/*****************************************************************************
 * @brief Fill an output file with data from an output stream.
 *
 * @param[in] sDataStream,    the output data stream to set.
 * @param[in] dataStreamName, the name of the data stream.
 * @param[in] outFileName,    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool getOutputDataStream(stream<UdpWord> &sDataStream,
                         const std::string  dataStreamName, const std::string outFileName, int simCnt);


bool dumpStringToFile(std::string s, const std::string outFileName, int simCnt);

unsigned char hexval(unsigned char c);

void hex2ascii(const std::string& in, std::string& out);

void ascii2hex(const std::string& in, std::string& out);

bool isCornerPresent(std::string str, std::string corner);

static inline ssize_t
__file_size(const char *fname);

static inline ssize_t
__file_read(const char *fname, char *buff, size_t len);

static inline ssize_t
__file_write(const char *fname, const char *buff, size_t len);

#endif

/*! \} */
