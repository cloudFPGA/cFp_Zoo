/*
 * Copyright 2019 Xilinx, Inc.
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
 */

/*****************************************************************************
 * @file       xf_harris_config.h
 * @brief      The Harris configuration header
 * @author     DID
 * @date       May 2020
 * 
 * @ingroup Harris
 * @addtogroup Harris
 * \{
 *****************************************************************************/


#ifndef _XF_HARRIS_CONFIG_H_
#define _XF_HARRIS_CONFIG_H_

#include "hls_stream.h"
#include "ap_int.h"
#include "common/xf_common.hpp"
#include "common/xf_utility.hpp"
#include "features/xf_harris.hpp"
#include "xf_config_params.h"
#include "../../../../../HOST/vision/harris/include/config.h"

// # for gammacorrection
#include "imgproc/xf_gammacorrection.hpp"
#if NO
#define NPC1 XF_NPPC1
#endif
#if RO
#define NPC1 XF_NPPC8
#endif


#define CH_TYPE XF_GRAY
#define INPUT_PTR_WIDTH 64
#define OUTPUT_PTR_WIDTH 64
/*
 * Pack pixels in and write into streams
 */

#if RO
#define NPIX XF_NPPC8
#endif
#if NO
#define NPIX XF_NPPC1
#endif

#define WIDTH FRAME_WIDTH
#define HEIGHT FRAME_HEIGHT

#define IMGSIZE FRAME_TOTAL

#define IMG_PACKETS IMGSIZE/(INPUT_PTR_WIDTH/8)

#define IN_TYPE XF_8UC1
#define OUT_TYPE XF_8UC1

void harris_accel(xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX>& _src,
                  xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX>& _dst,
                  unsigned short Thresh,
                  unsigned short k);

void cornerHarrisAccelArray(ap_uint<INPUT_PTR_WIDTH>* img_inp,
                        ap_uint<OUTPUT_PTR_WIDTH>* img_out,
                        int rows, int cols, int threshold, int k);

void cornerHarrisAccelStream(
    hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >& img_in_axi_stream,
    hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >& img_out_axi_stream,
    int rows, int cols, int threshold, int k);

void gammacorrection_accel(xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPC1>& imgInput1,
                           xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1>& imgOutput,
                           float gammaval);


#endif


/*! \} */