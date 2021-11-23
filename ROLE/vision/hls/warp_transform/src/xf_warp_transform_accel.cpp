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
 * @file       xf_warp_transform_accel.cpp
 * @brief      The WarpTransform top-level
 * @author     DCO
 * @date       Nov 2021
 * 
 * @ingroup WarpTransformHLS
 * @addtogroup WarpTransformHLS
 * \{
 *****************************************************************************/


#include "../include/xf_warp_transform_config.h"
#include <iostream>                     // For cout and cerr

using namespace std;


#ifndef FAKE_WarpTransform

/*****************************************************************************
 * @brief   Top-level accelerated function of the WarpTransform Application with 
 * array I/F
 * @ingroup WarpTransformHLS
 * add WARPTRANSFORM
 *
 * @return Nothing.
 *****************************************************************************/
//extern "C" {
void medianBlurAccelStream(
    hls::stream<ap_uint<INPUT_PTR_WIDTH>>& img_in_axi_stream,
    hls::stream<ap_uint<OUTPUT_PTR_WIDTH>>& img_out_axi_stream,
    int rows, int cols) {
    // clang-format on
    #pragma  HLS INLINE off

    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPIX> imgInput(rows, cols);
    // clang-format off
    #pragma HLS stream variable=imgInput.data depth=2
    // clang-format on

    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPIX> imgOutput(rows, cols);
    // clang-format off
    #pragma HLS stream variable=imgOutput.data depth=2
    // clang-format on

    // clang-format off
    #pragma HLS DATAFLOW
    // clang-format on

    accel_utils accel_utils_obj;
    
    int dstMat_cols_align_npc = ((imgInput.cols + (NPIX - 1)) >> XF_BITSHIFT(NPIX)) << XF_BITSHIFT(NPIX);

    accel_utils_obj.hlsStrm2xfMat<INPUT_PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPIX, (HEIGHT * WIDTH) / NPIX>(img_in_axi_stream, imgInput, dstMat_cols_align_npc);
    
    xf::cv::medianBlur<WINDOW_SIZE, XF_BORDER_REPLICATE, TYPE, HEIGHT, WIDTH, NPC1>(imgInput, imgOutput);    
    
    int srcMat_cols_align_npc = ((imgOutput.cols + (NPIX - 1)) >> XF_BITSHIFT(NPIX)) << XF_BITSHIFT(NPIX);
    
    accel_utils_obj.xfMat2hlsStrm<OUTPUT_PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPIX, HEIGHT*((WIDTH + NPIX - 1) / NPIX)>(imgOutput, img_out_axi_stream,
                                                                                        srcMat_cols_align_npc);    
}

#else // FAKE_WarpTransform
/*****************************************************************************
 * @brief   Top-level accelerated function of the WarpTransform Application with 
 * array I/F
 * @ingroup WarpTransformHLS
 *
 * @return Nothing.
 *****************************************************************************/
//extern "C" {
void fakeWarpTransformAccelStream(
    hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >& img_in_axi_stream,
    hls::stream<ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0> >& img_out_axi_stream,
    unsigned int min_rx_loops,
    unsigned int min_tx_loops) {

  #pragma  HLS INLINE off
  //#pragma HLS INTERFACE axis port=img_in_axi_stream
  //#pragma HLS INTERFACE axis port=img_out_axi_stream
  //#pragma HLS interface ap_ctrl_none port=return  // Special pragma for free-running kernel

  ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> tmp_in;
  ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0> tmp_out;
  for (unsigned int i=0, j=0, k=0; k < 5 * (min_rx_loops + min_tx_loops); k++) {
    cout << "Consuming input...i=" << i << endl;
    if (!img_in_axi_stream.empty() && (i < min_rx_loops)) {
      tmp_in = img_in_axi_stream.read();
      i++;
    }

    tmp_out.data = tmp_in.data; // known silent dirty casting here when INPUT_PTR_WIDTH != OUTPUT_PTR_WIDTH
    cout << "Filling output...j=" << j << endl;
    if (!(img_out_axi_stream.full()) && (j < min_tx_loops)) {
      img_out_axi_stream.write(tmp_out);
      j++;
    }
    //if ((img_out_axi_stream.full()) || (i == min_tx_loops)) {
    if (j == min_tx_loops) {
      cout << "Full" << endl;
      //break;
    }
  }
}

#endif // FAKE_WarpTransform



/*****************************************************************************
 * @brief   Top-level accelerated function of the WarpTransform Application with memory mapped interfaces
 * @ingroup WarpTransformHLS
 *
 * @return Nothing.
 *****************************************************************************/
//extern "C" {
void warp_transformAccelMem(         membus_t* img_inp,
                            membus_t* img_out1,
                            membus_t* img_out2,
                            int rows, int cols) {
    // clang-format on
    #pragma  HLS INLINE off

    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPIX> imgInput(rows, cols);
    // clang-format off
    #pragma HLS stream variable=imgInput.data depth=2
    // clang-format on

    #ifndef FAKE_WarpTransform
    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPIX> _dstgx(rows, cols);
    // clang-format off
    #pragma HLS stream variable=_dstgx.data depth=2
    // clang-format on
        xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPIX> _dstgy(rows, cols);
    // clang-format off
    #pragma HLS stream variable=_dstgy.data depth=2
    // clang-format on
    #endif
    
    // clang-format off
    #pragma HLS DATAFLOW
    // clang-format on

    // Feed a cv matrix from ddr memory
    xf::cv::Array2xfMat<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(img_inp, imgInput);
    
    #ifdef FAKE_WarpTransform
    // Feed ddr memory from a cv matrix
    xf::cv::xfMat2Array<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(imgInput, img_out1);
    // Feed ddr memory from a cv matrix
    xf::cv::xfMat2Array<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(imgInput, img_out2);
    
    #else
    
    // xf::cv::medianBlur<WINDOW_SIZE, XF_BORDER_REPLICATE, TYPE, HEIGHT, WIDTH, NPC1>(imgInput, imgOutput);    
    xf::cv::WarpTransform<XF_BORDER_CONSTANT, FILTER_WIDTH,
              IN_TYPE, TYPE, HEIGHT, WIDTH,
              NPC1, XF_USE_URAM>(in_mat, _dstgx,_dstgy);

    // Feed ddr memory from a cv matrix
    xf::cv::xfMat2Array<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(_dstgx, img_out1);
    // Feed ddr memory from a cv matrix
    xf::cv::xfMat2Array<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(_dstgy, img_out2);
    
    #endif
    
    
}



/*! \} */
