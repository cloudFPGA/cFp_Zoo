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
 * @file       xf_median_blur_config.h
 * @brief      The MedianBlur configuration header
 * @author     DID
 * @date       May 2020
 * 
 * @ingroup MedianBlur
 * @addtogroup MedianBlur
 * \{
 *****************************************************************************/


#ifndef _XF_MEDIANBLUR_CONFIG_H_
#define _XF_MEDIANBLUR_CONFIG_H_

#include "hls_stream.h"
#include "ap_int.h"
#include "common/xf_common.hpp"
#include "common/xf_utility.hpp"
#include "imgproc/xf_median_blur.hpp"
#include "xf_config_params.h"
#include "median_blur.hpp"
#include "../../../../../HOST/vision/median_blur/languages/cplusplus/include/config.h"

#ifdef USE_HLSLIB_STREAM
#include "../../../../../hlslib/include/hlslib/xilinx/Stream.h"
using hlslib::Stream;
#endif
using hls::stream;

// Set the optimization type:
#if NO == 1
#define NPC1 XF_NPPC1
#define PTR_WIDTH 128
#else

#if GRAY
#define NPC1 XF_NPPC8
#else
#define NPC1 XF_NPPC4
#endif

#define PTR_WIDTH 128
#endif

// Set the pixel depth:
#if GRAY
#define TYPE XF_8UC1
#else
#define TYPE XF_8UC3
#endif

#define CH_TYPE XF_GRAY
#define INPUT_PTR_WIDTH 8
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

#define BITS_PER_10GBITETHRNET_AXI_PACKET 64
#define BYTES_PER_10GBITETHRNET_AXI_PACKET (BITS_PER_10GBITETHRNET_AXI_PACKET/8)

#define IMG_PACKETS IMGSIZE/(BYTES_PER_10GBITETHRNET_AXI_PACKET)

#define MIN_RX_LOOPS IMG_PACKETS*(BITS_PER_10GBITETHRNET_AXI_PACKET/INPUT_PTR_WIDTH)
#define MIN_TX_LOOPS IMG_PACKETS*(BITS_PER_10GBITETHRNET_AXI_PACKET/OUTPUT_PTR_WIDTH)

// Compatibility for common.hpp
#define OUT_TYPE TYPE

// Enable it to use the dataflow mode of hlslib
// #define USE_HLSLIB_DATAFLOW

// Enable it to use the Stream class of hlslib
// #define USE_HLSLIB_STREAM

// Enable it to fake the call of actual MedianBlur kernel and instead consume input data and write back 
// the last element from the input to every output value. This option is used for debugging.
// #define FAKE_MedianBlur

// Function prototypes

void median_blur_accel( xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX>& _src,
                        xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX>& _dst);

void medianBlurAccelArray(  ap_uint<INPUT_PTR_WIDTH>* img_inp,
                            ap_uint<OUTPUT_PTR_WIDTH>* img_out,
                            int rows, int cols);

void medianBlurAccelStream(
    hls::stream<ap_uint<INPUT_PTR_WIDTH>>& img_in_axi_stream,
    hls::stream<ap_uint<OUTPUT_PTR_WIDTH>>& img_out_axi_stream,
    int rows, int cols);

void fakeMedianBlurAccelStream(
    #ifdef USE_HLSLIB_STREAM
    hlslib::Stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0>, MIN_RX_LOOPS>        & img_in_axi_stream,
    hlslib::Stream<ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0>, MIN_TX_LOOPS>       & img_out_axi_stream,
    #else
    hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >& img_in_axi_stream,
    hls::stream<ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0> >& img_out_axi_stream,
    #endif
    unsigned int min_rx_loops,
    unsigned int min_tx_loops);

void medianBlurAccelMem(    membus_t* img_inp,
                            membus_t* img_out,
                            int rows, int cols);


#endif // end of _XF_MEDIAN_BLUR_CONFIG_H_


/*! \} */
