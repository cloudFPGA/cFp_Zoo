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
 * @file       xf_harris_accel.cpp
 * @brief      The Harris top-level
 * @author     DID
 * @date       May 2020
 * 
 * @ingroup HarrisHLS
 * @addtogroup HarrisHLS
 * \{
 *****************************************************************************/


#include "../include/xf_harris_config.h"
#include <iostream>                     // For cout and cerr

using namespace std;

/*****************************************************************************
 * @brief   Top-level accelerated function of the Harris Application with 
 * xf::cv I/F
 * @ingroup HarrisHLS
 *
 * @return Nothing.
 *****************************************************************************/
void harris_accel(xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX>& _src,
                  xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX>& _dst,
                  unsigned short Thresh,
                  unsigned short k) {
    xf::cv::cornerHarris<FILTER_WIDTH, BLOCK_WIDTH, NMS_RADIUS, XF_8UC1, HEIGHT, WIDTH, NPIX, XF_USE_URAM>(_src, _dst,
                                                                                                           Thresh, k);
}


/*****************************************************************************
 * @brief   Top-level accelerated function of the Harris Application with 
 * array I/F
 * @ingroup HarrisHLS
 *
 * @return Nothing.
 *****************************************************************************/
//extern "C" {
void cornerHarrisAccelArray(
    ap_uint<INPUT_PTR_WIDTH>* img_inp, ap_uint<OUTPUT_PTR_WIDTH>* img_out, int rows, int cols, int threshold, int k) {
// clang-format off
/*    #pragma HLS INTERFACE m_axi     port=img_inp  offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi     port=img_out  offset=slave bundle=gmem2

    #pragma HLS INTERFACE s_axilite port=rows     bundle=control
    #pragma HLS INTERFACE s_axilite port=cols     bundle=control
    #pragma HLS INTERFACE s_axilite port=threshold     bundle=control
    #pragma HLS INTERFACE s_axilite port=k     bundle=control
    #pragma HLS INTERFACE s_axilite port=return   bundle=control
    // clang-format on
*/
    const int pROWS = HEIGHT;
    const int pCOLS = WIDTH;
    const int pNPC1 = NPIX;

    xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX> in_mat(rows, cols);
// clang-format off
    #pragma HLS stream variable=in_mat.data depth=2
    // clang-format on

    xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX> out_mat(rows, cols);
// clang-format off
    #pragma HLS stream variable=out_mat.data depth=2
// clang-format on

// clang-format off
    #pragma HLS DATAFLOW
    // clang-format on
    xf::cv::Array2xfMat<INPUT_PTR_WIDTH, XF_8UC1, HEIGHT, WIDTH, NPIX>(img_inp, in_mat);
    xf::cv::cornerHarris<FILTER_WIDTH, BLOCK_WIDTH, NMS_RADIUS, XF_8UC1, HEIGHT, WIDTH, NPIX, XF_USE_URAM>(
        in_mat, out_mat, threshold, k);
    xf::cv::xfMat2Array<OUTPUT_PTR_WIDTH, XF_8UC1, HEIGHT, WIDTH, NPIX>(out_mat, img_out);
}
//}


#ifndef FAKE_Harris

/*****************************************************************************
 * @brief   Top-level accelerated function of the Harris Application with 
 * array I/F
 * @ingroup HarrisHLS
 *
 * @return Nothing.
 *****************************************************************************/
//extern "C" {
void cornerHarrisAccelStream(
    hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >& img_in_axi_stream,
    hls::stream<ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0> >& img_out_axi_stream,
    int rows, int cols, int threshold, int k) {
    // clang-format on

    const int pROWS = HEIGHT;
    const int pCOLS = WIDTH;
    const int pNPC1 = NPIX;

    xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPIX> in_mat(rows, cols);
    // clang-format off
    #pragma HLS stream variable=in_mat.data depth=2
    // clang-format on

    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPIX> out_mat(rows, cols);
    // clang-format off
    #pragma HLS stream variable=out_mat.data depth=2
    // clang-format on

    // clang-format off
    #pragma HLS DATAFLOW
    // clang-format on

    xf::cv::axiStrm2xfMat<INPUT_PTR_WIDTH, IN_TYPE, HEIGHT, WIDTH, NPIX>(
      img_in_axi_stream, in_mat);  
    xf::cv::cornerHarris<FILTER_WIDTH, BLOCK_WIDTH, NMS_RADIUS, IN_TYPE, HEIGHT, WIDTH, NPIX, XF_USE_URAM>(
      in_mat, out_mat, threshold, k);
    //float gammaval = 0.2;
    //xf::cv::gammacorrection<IN_TYPE, OUT_TYPE, HEIGHT, WIDTH, NPC1>(in_mat, out_mat, gammaval);    
    xf::cv::xfMat2axiStrm<OUTPUT_PTR_WIDTH, OUT_TYPE, HEIGHT, WIDTH, NPIX>(
      out_mat, img_out_axi_stream);
    
    
}

#else // FAKE_Harris
/*****************************************************************************
 * @brief   Top-level accelerated function of the Harris Application with 
 * array I/F
 * @ingroup HarrisHLS
 *
 * @return Nothing.
 *****************************************************************************/
//extern "C" {
void fakeCornerHarrisAccelStream(
    hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >& img_in_axi_stream,
    hls::stream<ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0> >& img_out_axi_stream,
    unsigned int min_rx_loops,
    unsigned int min_tx_loops) {
  
  ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> tmp_in;
  ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0> tmp_out;
  for (unsigned int i=0; i<min_rx_loops; i++) { 
    cout << "Consuming input...i=" << i << endl;
    if ((img_in_axi_stream.empty()) || (i == min_rx_loops-1)) {
      cout << "Empty" << endl;
      break;
    }
    tmp_in = img_in_axi_stream.read();  
  }
  tmp_out.data = tmp_in.data; // known silent dirty casting here when INPUT_PTR_WIDTH != OUTPUT_PTR_WIDTH
  for (unsigned int i=0; i<min_tx_loops; i++) { 
    cout << "Filling output...i=" << i << endl;
    if ((img_out_axi_stream.full()) || (i == min_tx_loops-1)) {
      cout << "Full" << endl;
      break;
    }
    img_out_axi_stream.write(tmp_out);  
  }
}

#endif // FAKE_Harris

/*! \} */
