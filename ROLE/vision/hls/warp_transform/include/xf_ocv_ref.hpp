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
 * @file       xf_ocv_ref.hpp
 * @brief      The WarpTransform reference functions used in TB
 * @author     DID,DCO
 * @date       Nov 2021
 * 
 * @ingroup WarpTransformTB
 * @addtogroup WarpTransformTB
 * \{
 *****************************************************************************/


#ifndef _XF_OCV_REF_HPP_
#define _XF_OCV_REF_HPP_
#include "xf_config_params.h"

using namespace cv;
using namespace std;


void ocv_ref(cv::Mat image_input, cv::Mat& opencv_image, cv::Mat transformation_matrix) {

    for (int I1 = 0; I1 < opencv_image.rows; I1++) {
        for (int J1 = 0; J1 < opencv_image.cols; J1++) {
#if GRAY
            opencv_image.at<unsigned char>(I1, J1) = 0;
#else
            opencv_image.at<cv::Vec3b>(I1, J1) = 0;
#endif
        }
    }
#ifdef FAKE_WarpTransform
    image_input.copyTo(opencv_image);
#else
#if TRANSFORM_TYPE == 1
#if INTERPOLATION == 1
    cv::warpPerspective(image_input, opencv_image, transformation_matrix,
                        cv::Size(image_input.cols, image_input.rows), cv::INTER_LINEAR + cv::WARP_INVERSE_MAP,
                        cv::BORDER_TRANSPARENT, 80);
#else
    cv::warpPerspective(image_input, opencv_image, transformation_matrix,
                        cv::Size(image_input.cols, image_input.rows), cv::INTER_NEAREST + cv::WARP_INVERSE_MAP,
                        cv::BORDER_TRANSPARENT, 80);
#endif//INTERPOLATION == 1
#else
#if INTERPOLATION == 1
    cv::warpAffine(image_input, opencv_image, transformation_matrix, cv::Size(image_input.cols, image_input.rows),
                   cv::INTER_LINEAR + cv::WARP_INVERSE_MAP, cv::BORDER_TRANSPARENT, 80);
#else
    cv::warpAffine(image_input, opencv_image, transformation_matrix, cv::Size(image_input.cols, image_input.rows),
                   cv::INTER_NEAREST + cv::WARP_INVERSE_MAP, cv::BORDER_TRANSPARENT, 80);
#endif//INTERPOLATION == 1
#endif//TRANSFORM_TYPE == 1
#endif //FAKE_WarpTransform

    cv::imwrite("opencv_output.png", opencv_image);
}


#endif //_XF_OCV_REF_HPP_


/*! \} */
