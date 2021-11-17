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
 * @file       xf_sobel_accel.cpp
 * @brief      The Sobel top-level
 * @author     DCO
 * @date       Nov 2021
 * 
 * @ingroup SobelHLS
 * @addtogroup SobelHLS
 * \{
 *****************************************************************************/


#include "../include/xf_sobel_config.h"
#include <iostream>                     // For cout and cerr

using namespace std;

/*****************************************************************************
 * @brief   Top-level accelerated function of the Sobel Application with 
 * xf::cv I/F
 * @ingroup SobelHLS
 *
 * @return Nothing.
 *****************************************************************************/
void sobel_accel( xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1>& imgInput,
                        xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1>& imgOutput) {
    
    // Run xfOpenCV kernel:
    xf::cv::medianBlur<WINDOW_SIZE, XF_BORDER_REPLICATE, TYPE, HEIGHT, WIDTH, NPC1>(imgInput, imgOutput);    

}


/*****************************************************************************
 * @brief   Top-level accelerated function of the Sobel Application with 
 * array I/F
 * @ingroup SobelHLS
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


#ifndef FAKE_Sobel

/*****************************************************************************
 * @brief   Top-level accelerated function of the Sobel Application with 
 * array I/F
 * @ingroup SobelHLS
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

#else // FAKE_Sobel
/*****************************************************************************
 * @brief   Top-level accelerated function of the Sobel Application with 
 * array I/F
 * @ingroup SobelHLS
 *
 * @return Nothing.
 *****************************************************************************/
//extern "C" {
void fakeSobelAccelStream(
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

#endif // FAKE_Sobel



/*****************************************************************************
 * @brief   Top-level accelerated function of the Sobel Application with 
 * array I/F
 * @ingroup SobelHLS
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

    #ifndef FAKE_Sobel
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
    
    #ifdef FAKE_Sobel
    
    // Feed ddr memory from a cv matrix
    xf::cv::xfMat2Array<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(imgInput, img_out);
    #else
    
    xf::cv::medianBlur<WINDOW_SIZE, XF_BORDER_REPLICATE, TYPE, HEIGHT, WIDTH, NPC1>(imgInput, imgOutput);    

    // Feed ddr memory from a cv matrix
    xf::cv::xfMat2Array<MEMDW_512, XF_8UC1, HEIGHT, WIDTH, NPIX>(imgOutput, img_out);
    
    #endif
    
    
}



/*! \} */
