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
 * @file       commom.hpp
 * @brief      Common functions for testbenches - headers.
 *
 * @date       September 2021
 * @author     FAB, WEI, NGL, DID, DCO
 * 
 * Copyright 2009-2015 - Xilinx Inc.  - All rights reserved.
 * Copyright 2015-2021 - IBM Research - All Rights Reserved.
 *
 * @ingroup IBMZRL 
 * @addtogroup IBMZRL 
 * \{
 *****************************************************************************/


#ifndef _TEST_COMMON_H_
#define _TEST_COMMON_H_


#include <stdio.h>
#include <string>
#include <hls_stream.h>
#include <sys/stat.h>
#include <cstdlib>                      // For atoi()
#include <string>                       // For to_string
#include <cstring>                       
#include <string.h>
#include <bitset>
#ifdef ROLE_IS_UPPERCASE
#include "../../../../../HOST/custom/uppercase/languages/cplusplus/include/config.h"
#include "../../uppercase/include/uppercase.hpp"
#endif //ROLE_IS_UPPERCASE

#ifdef ROLE_IS_MEMTEST
#include "../../memtest/include/memtest.hpp"
#include "../../../../../HOST/custom/memtest/languages/cplusplus/include/config.h"
#endif //ROLE_IS_MEMTEST

#include <bits/stdc++.h>
#include <typeinfo>

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


//Data structure of a memory test Result
struct MemoryTestResult {
  unsigned long long int    target_address;
  unsigned int    fault_cntr;
  unsigned int    first_fault_address;
  unsigned long long int    clock_cycles_read;
  unsigned long long int    clock_cycles_write;

  MemoryTestResult()      {}
  MemoryTestResult(
    unsigned long long int target_address,
    unsigned int fault_cntr,
    unsigned int  first_fault_address,
    unsigned long long int clock_cycles_write,
    unsigned long long int clock_cycles_read) :
    target_address(target_address), 
    fault_cntr(fault_cntr), 
    first_fault_address(first_fault_address),  
    clock_cycles_write(clock_cycles_write),
    clock_cycles_read(clock_cycles_read) {}
};

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
 * @brief Fill an output file with data from an output stream.
 *
 * @param[in] sDataStream,    the output data stream to set.
 * @param[in] dataStreamName, the name of the data stream.
 * @param[in] outFileName,    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool getOutputDataStream(stream<UdpWord> &sDataStream,
                         const std::string  dataStreamName, const std::string outFileName, int simCnt);

/*****************************************************************************
 * @brief Initialize an input data stream from a file.
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] strOutput the output string to set.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool dumpStringToFile(std::string s, const std::string outFileName, int simCnt);

/*****************************************************************************
 * @brief Fill an output file with data from an image.
 * 
 * 
 * @param[in] sDataStream    the input image in xf::cv::Mat format.
 * @param[in] outFileName    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool dumpStringToFileOnlyRawData(const std::string s, const std::string   outFileName, int simCnt, size_t out_size);

/*****************************************************************************
 * @brief Fill an output file with data from a string and
 *          set the tlast every gno packets
 * 
 * 
 * @param[in] sDataStream    the input image in xf::cv::Mat format.
 * @param[in] outFileName    the name of the output file to write to.
 * @param[in] simCnt         
 * @param[in] gno            the counter value at which this function set the tlast=1
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool dumpStringToFileWithLastSetEveryGnoPackets(std::string s, const std::string   outFileName, int simCnt, int gno);

/*****************************************************************************
 * @brief Initialize an input data stream from a file.
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] strOutput the output string to set.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool dumpFileToString(const std::string inpFileName, std::string strOutput, int simCnt);

/*****************************************************************************
 * @brief Initialize an input data stream from a file with only data
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] strOutput the output string to set.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
template<unsigned int bytes_per_line = 8>
std::string dumpFileToStringRawDataString(const std::string inpFileName, int * rawdatalines, size_t outputSize);

/*****************************************************************************
 * @brief convert a char to its hexadecimal representation.
 *
 * @param[in]  c the standard char value to convert to hex
 * @return     the hexadecimal value of the input
 ******************************************************************************/
unsigned char hexval(unsigned char c);

/*****************************************************************************
 * @brief Convert a hexadecimal string to a ascii string
 *
 * @param[in]  in the input hexadecimal string
 * @param[out] out the output ascii string
 ******************************************************************************/
void hex2ascii(const std::string& in, std::string& out);

/*****************************************************************************
 * @brief Convert a ascii string to a hexadecimal string
 *
 * @param[in]  in the input ascii string
 * @param[out] out the output hexadecimal string
 ******************************************************************************/
void ascii2hex(const std::string& in, std::string& out);

/*****************************************************************************
 * @brief Convert a ascii string to a hexadecimal string
 *
 * @param[in]  in the input ascii string
 * @param[out] out the output hexadecimal string
 * @param[in]  bytesize the input ascii string size
 ******************************************************************************/
void ascii2hexWithSize(const std::string& in, std::string& out, size_t  bytesize);

/*****************************************************************************
 * @brief Check the presence of a given corner value at the begin and the end of a string
 *
 * @param[in]  str the input string to be checked
 * @param[in]  corner the corner char to find
 * @return true if it is present, false otherwise
 ******************************************************************************/
bool isCornerPresent(std::string str, std::string corner);

/*****************************************************************************
 * @brief Convert a hex string to a T type into a char buffer with the SAME dimensions
 *
 * @param[in]  in the input hex string
 * @param[out] out the output numerical hexadec string string
 * @param[in]  byteSize the bytesize of the input string and the buffer, it assumes equal dimension
 ******************************************************************************/
template<typename T>
void string2hexnumerics(const std::string& in, char * out, size_t byteSize);

/*****************************************************************************
 * @brief Create the commands for a memory test with start/max address to test-nop to execute-stop
 *
 * @param[in]  mem_address max target address to test
 * @param[in]  testingNumber the number of tests to perform on the memory
 * 
 * @return out the output string with start/max address-nops4trgtCCsNeeded-stop
 ******************************************************************************/
std::string createMemTestCommands(unsigned long long int mem_address, int testingNumber, unsigned int burst_size);

/*****************************************************************************
 * @brief Create the expected output results for the memory test (with FAULT INJECTION)
 *
 * @param[in]  mem_address max target address to test
 * @param[in]  testingNumber the number of tests to perform on the memory
 * 
 * @return  out the results of the memory test (with FAULT INJECTION)
 ******************************************************************************/
std::string createMemTestGoldenOutput(unsigned long long int mem_address, int testingNumber);

/*****************************************************************************
 * @brief Create the expected output results for the uppercase
 *
 * @param[in]  input_string the input string of the uppercase
 * 
 * @return  out the results of the uppercase
 ******************************************************************************/
std::string createUppercaseGoldenOutput(std::string input_string);

/*****************************************************************************
 * @brief reverse a given string
 * 
 * @param[in]  str the string to reverse.
 * @return nothing, print to stdout.
 ******************************************************************************/
void reverseStr(std::string& str);

/*****************************************************************************
 * @brief Parse the memory test output contained in astring with a given size
 * 
 * @param[in]  longbuf the buffer containing the output
 * @param[in] charOutputSize the bytesize of the buffer
 * @param[in] rawdatalines the number of lines in the given outbuf
 * @return vectpr of MemoryTestResult data strcuture
 ******************************************************************************/
template<unsigned int bytes_per_line=8>
std::vector<MemoryTestResult> parseMemoryTestOutput(const std::string longbuf, size_t charOutputSize, int rawdatalines);

/*****************************************************************************
 * @brief print byte-per-byte a given string in hexadecimal format
 *
 * @param[in]  inStr string to print
 * @param[in]  strSize bytsize to print (can be even less, NOT more )
 * 
 ******************************************************************************/
void printStringHex(const std::string inStr, size_t strSize);

/*****************************************************************************
 * @brief print byte-per-byte a given char buff in hexadecimal format
 *
 * @param[in]  inStr char buff to print
 * @param[in]  strSize bytsize to print (can be even less, NOT more )
 * 
 ******************************************************************************/
void printCharBuffHex(const char * inStr, size_t strSize);

/*****************************************************************************
 * @brief print the binary representation of a target pointer buffer of a given size.
 *      Assumes little endian.
 * @param[in]  size the bytesize to print from ptr.
 * @param[in] ptr the buffer pointer.
 * @return nothing, print to stdout.
 ******************************************************************************/
void printBits(size_t const size, void const * const ptr);

static inline ssize_t
__file_size(const char *fname);

static inline ssize_t
__file_read(const char *fname, char *buff, size_t len);

static inline ssize_t
__file_write(const char *fname, const char *buff, size_t len);

#endif

/*! \} */
