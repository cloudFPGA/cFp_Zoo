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
    hls::stream<ap_uint<INPUT_PTR_WIDTH>>& img_in_axi_stream,
    //hls::stream<ap_axiu<INPUT_PTR_WIDTH, 0, 0, 0> >& img_in_axi_stream,
    hls::stream<ap_uint<OUTPUT_PTR_WIDTH>>& img_out_axi_stream,
    //hls::stream<ap_axiu<OUTPUT_PTR_WIDTH, 0, 0, 0> >& img_out_axi_stream,
    int rows, int cols, int threshold, int k) {
    // clang-format on
    #pragma  HLS INLINE off
    
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

    accel_utils accel_utils_obj;
    
    int dstMat_cols_align_npc = ((in_mat.cols + (NPIX - 1)) >> XF_BITSHIFT(NPIX)) << XF_BITSHIFT(NPIX);

    accel_utils_obj.hlsStrm2xfMat<INPUT_PTR_WIDTH, IN_TYPE, HEIGHT, WIDTH, NPIX, (HEIGHT * WIDTH) / NPIX>(img_in_axi_stream, in_mat, dstMat_cols_align_npc);
    
    //xf::cv::axiStrm2xfMat<INPUT_PTR_WIDTH, IN_TYPE, HEIGHT, WIDTH, NPIX>(
    //  img_in_axi_stream, in_mat);  

    xf::cv::cornerHarris<FILTER_WIDTH, BLOCK_WIDTH, NMS_RADIUS, IN_TYPE, HEIGHT, WIDTH, NPIX, XF_USE_URAM>(
      in_mat, out_mat, threshold, k);

    //xf::cv::xfMat2axiStrm<OUTPUT_PTR_WIDTH, OUT_TYPE, HEIGHT, WIDTH, NPIX>(
    //  out_mat, img_out_axi_stream);
    
    int srcMat_cols_align_npc = ((out_mat.cols + (NPIX - 1)) >> XF_BITSHIFT(NPIX)) << XF_BITSHIFT(NPIX);
    
    accel_utils_obj.xfMat2hlsStrm<OUTPUT_PTR_WIDTH, OUT_TYPE, HEIGHT, WIDTH, NPIX, HEIGHT*((WIDTH + NPIX - 1) / NPIX)>(out_mat, img_out_axi_stream,
                                                                                        srcMat_cols_align_npc);
    
    
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

#endif // FAKE_Harris



/*****************************************************************************
 * @brief   Top-level accelerated function of the Harris Application with 
 * array I/F
 * @ingroup HarrisHLS
 *
 * @return Nothing.
 *****************************************************************************/
//extern "C" {
void cornerHarrisAccelMem(membus_t* img_inp,
                          membus_t* img_out,
                          int rows, int cols, int threshold, int k) {
    // clang-format on
    #pragma  HLS INLINE off
    
    const int pROWS = HEIGHT;
    const int pCOLS = WIDTH;
    const int pNPC1 = NPIX;

    xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPIX> in_mat(rows, cols);
    // clang-format off
    #pragma HLS stream variable=in_mat.data depth=2
    // clang-format on

    #ifndef FAKE_Harris
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPIX> out_mat(rows, cols);
    // clang-format off
    #pragma HLS stream variable=out_mat.data depth=2
    // clang-format on
    #endif
    
    // clang-format off
    #pragma HLS DATAFLOW
    // clang-format on

    // Feed a cv matrix from ddr memory
    xf::cv::Array2xfMat<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(img_inp, in_mat);
    
    #ifdef FAKE_Harris
    
    // Feed ddr memory from a cv matrix
    xf::cv::xfMat2Array<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(in_mat, img_out);
    #else
    
    xf::cv::cornerHarris<FILTER_WIDTH, BLOCK_WIDTH, NMS_RADIUS, IN_TYPE, HEIGHT, WIDTH, NPIX, XF_USE_URAM>(
      in_mat, out_mat, threshold, k);
    
    // Feed ddr memory from a cv matrix
    xf::cv::xfMat2Array<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(out_mat, img_out);
    
    #endif
    
    
}



/*! \} */
