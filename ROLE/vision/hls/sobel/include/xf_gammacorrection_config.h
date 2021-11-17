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
 * @file       xf_sobel_config.h
 * @brief      The Sobel configuration header
 * @author     DCO
 * @date       Nov 2021
 * 
 * @ingroup Sobel
 * @addtogroup Sobel
 * \{
 *****************************************************************************/


#ifndef _XF_GAMMA_CONFIG_H_
#define _XF_GAMMA_CONFIG_H_

#include "hls_stream.h"
#include "ap_int.h"
#include "common/xf_common.hpp"
#include "common/xf_utility.hpp"
#include "features/xf_sobel.hpp"
#include "xf_config_params.h"
#include "../../../host/sobel/include/config.h"
#include "imgproc/xf_gammacorrection.hpp"

#define WIDTH FRAME_WIDTH
#define HEIGHT FRAME_HEIGHT

#define IMGSIZE WIDTH * HEIGHT
#define IMG_PACKETS IMGSIZE/(INPUT_PTR_WIDTH/8)

#if NO
#define NPC1 XF_NPPC1
#endif
#if RO
#define NPC1 XF_NPPC8
#endif

//#define IN_TYPE XF_8UC3
//#define OUT_TYPE XF_8UC3


#if RO
#define IN_TYPE ap_uint<64>
#endif
#if NO
#define IN_TYPE ap_uint<8>
#endif


void gammacorrection_accel(xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPC1>& imgInput1,
                           xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1>& imgOutput,
                           float gammaval);


void cornerGammacorrectionAccelStream(
    hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >& img_in_axi_stream,
    hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >& img_out_axi_stream,
    float gammaval);

#endif //_XF_GAMMA_CONFIG_H_


/*! \} */