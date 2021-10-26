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
#ifdef ROLE_IS_HARRIS
#include "../../harris/include/harris.hpp"
#endif
#ifdef ROLE_IS_MEDIANBLUR
#include "../../median_blur/include/median_blur.hpp"
#endif

#include "common/xf_headers.hpp"

#ifdef ROLE_IS_HARRIS
#include "../../harris/include/xf_harris_config.h"
#include "../../harris/include/xf_ocv_ref.hpp"
#endif
#ifdef ROLE_IS_MEDIANBLUR
#include "../../median_blur/include/xf_median_blur_config.h"
#include "../../median_blur/include/xf_ocv_ref.hpp"
#endif


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
 * @brief Initialize an input array from a file with format "tdata tkeep tlast"
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] imgOutputArray the array to write the tdata only field from the file.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool setInputFileToArray(const std::string inpFileName, ap_uint<OUTPUT_PTR_WIDTH>* imgArray, int simCnt);


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
 * @brief Fill an output file with data from an image.
 * 
 * @param[in] sDataStream    the input image in xf::cv::Mat format.
 * @param[in] outFileName    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool dumpImgToFile(xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPIX>& _img,
		   const std::string outFileName, int simCnt);


/*****************************************************************************
 * @brief Write the corners found by Harris into a file.
 * 
 * @return 0 if successful, otherwise 1.
 ******************************************************************************/
unsigned int writeCornersIntoFile(cv::Mat& in_img, cv::Mat& ocv_out_img, cv::Mat& out_img, 
		             std::vector<cv::Point>& hls_points,
			     std::vector<cv::Point>& ocv_points,
			     std::vector<cv::Point>& common_pts);


/*****************************************************************************
 * @brief Mark the points found by Harris into the image.
 * 
 * @return Nothing
 ******************************************************************************/
void markPointsOnImage(xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPIX>& imgOutput, cv::Mat& in_img,
		       cv::Mat& out_img, std::vector<cv::Point>& hls_points);


#endif

/*! \} */
