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

/*****************************************************************************************************
Copyright (c) 2019, Xilinx, Inc.

All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************************************/

/*****************************************************************************
 * @file       xf_median_blur_accel.cpp
 * @brief      The MedianBlur top-level
 * @author     DID
 * @date       May 2020
 * 
 * @ingroup MedianBlurHLS
 * @addtogroup MedianBlurHLS
 * \{
 *****************************************************************************/


#include "../include/xf_median_blur_config.h"
#include <iostream>                     // For cout and cerr

using namespace std;

/*****************************************************************************
 * @brief   Top-level accelerated function of the MedianBlur Application with 
 * xf::cv I/F
 * @ingroup MedianBlurHLS
 *
 * @return Nothing.
 *****************************************************************************/
void median_blur_accel( xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1>& imgInput,
                        xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1>& imgOutput) {
    
    // Run xfOpenCV kernel:
    xf::cv::medianBlur<WINDOW_SIZE, XF_BORDER_REPLICATE, TYPE, HEIGHT, WIDTH, NPC1>(imgInput, imgOutput);    

}


/*****************************************************************************
 * @brief   Top-level accelerated function of the MedianBlur Application with 
 * array I/F
 * @ingroup MedianBlurHLS
 *
 * @return Nothing.
 *****************************************************************************/
//extern "C" {
void medianBlurAccelArray(
    ap_uint<INPUT_PTR_WIDTH>* img_in, ap_uint<OUTPUT_PTR_WIDTH>* img_out, int rows, int cols) {
// clang-format off
//    #pragma HLS INTERFACE m_axi      port=img_in        offset=slave  bundle=gmem0 depth=__XF_DEPTH
//    #pragma HLS INTERFACE m_axi      port=img_out        offset=slave bundle=gmem1 depth=__XF_DEPTH
//    #pragma HLS INTERFACE s_axilite  port=rows 			       	   	  bundle=control
//    #pragma HLS INTERFACE s_axilite  port=cols 			              bundle=contro
//    #pragma HLS INTERFACE s_axilite  port=return 			          bundle=control
// clang-format on
    
    const int pROWS = HEIGHT;
    const int pCOLS = WIDTH;
    const int pNPC1 = NPIX;

    xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX> imgInput(rows, cols);
// clang-format off
    #pragma HLS stream variable=imgInput.data depth=2
    // clang-format on

    xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX> imgOutput(rows, cols);
// clang-format off
    #pragma HLS stream variable=imgOutput.data depth=2
// clang-format on

// clang-format off
    #pragma HLS DATAFLOW
    // clang-format on
    xf::cv::Array2xfMat<INPUT_PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPC1>(img_in, imgInput);
    xf::cv::medianBlur<WINDOW_SIZE, XF_BORDER_REPLICATE, TYPE, HEIGHT, WIDTH, NPC1>(imgInput, imgOutput);    
    xf::cv::xfMat2Array<OUTPUT_PTR_WIDTH, XF_8UC1, HEIGHT, WIDTH, NPIX>(imgOutput, img_out);
}
//}


#ifndef FAKE_MedianBlur

/*****************************************************************************
 * @brief   Top-level accelerated function of the MedianBlur Application with 
 * array I/F
 * @ingroup MedianBlurHLS
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

#else // FAKE_MedianBlur
/*****************************************************************************
 * @brief   Top-level accelerated function of the MedianBlur Application with 
 * array I/F
 * @ingroup MedianBlurHLS
 *
 * @return Nothing.
 *****************************************************************************/
//extern "C" {
void fakeMedianBlurAccelStream(
    #ifdef USE_HLSLIB_STREAM
    hlslib::Stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0>, MIN_RX_LOOPS>        &img_in_axi_stream,
    hlslib::Stream<ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0>, MIN_TX_LOOPS>       &img_out_axi_stream,
    #else
    hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >& img_in_axi_stream,
    hls::stream<ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0> >& img_out_axi_stream,
    #endif
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

#endif // FAKE_MedianBlur



/*****************************************************************************
 * @brief   Top-level accelerated function of the MedianBlur Application with 
 * array I/F
 * @ingroup MedianBlurHLS
 *
 * @return Nothing.
 *****************************************************************************/
//extern "C" {
void medianBlurAccelMem(    membus_t* img_inp,
                            membus_t* img_out,
                            int rows, int cols) {
    // clang-format on
    #pragma  HLS INLINE off

    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPIX> imgInput(rows, cols);
    // clang-format off
    #pragma HLS stream variable=imgInput.data depth=2
    // clang-format on

    #ifndef FAKE_MedianBlur
    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPIX> imgOutput(rows, cols);
    // clang-format off
    #pragma HLS stream variable=imgOutput.data depth=2
    // clang-format on
    #endif
    
    // clang-format off
    #pragma HLS DATAFLOW
    // clang-format on

    // Feed a cv matrix from ddr memory
    xf::cv::Array2xfMat<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(img_inp, imgInput);
    
    #ifdef FAKE_MedianBlur
    
    // Feed ddr memory from a cv matrix
    xf::cv::xfMat2Array<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(imgInput, img_out);
    #else
    
    xf::cv::medianBlur<WINDOW_SIZE, XF_BORDER_REPLICATE, TYPE, HEIGHT, WIDTH, NPC1>(imgInput, imgOutput);    

    // Feed ddr memory from a cv matrix
    xf::cv::xfMat2Array<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(imgOutput, img_out);
    
    #endif
    
    
}



/*! \} */
