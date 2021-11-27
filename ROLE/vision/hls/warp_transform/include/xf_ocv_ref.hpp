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
            opencv_image.at<ap_uint8_t>(I1, J1) = 0;
#else
            opencv_image.at<cv::Vec3b>(I1, J1) = 0;
#endif
        }
    }

#if TRANSFORM_TYPE == 1
#if INTERPOLATION == 1
    cv::warpPerspective(image_input, opencv_image, transformation_matrix,
                        cv::Size(image_input.cols, image_input.rows), cv::INTER_LINEAR + cv::WARP_INVERSE_MAP,
                        cv::BORDER_TRANSPARENT, 80);
#else
    cv::warpPerspective(image_input, opencv_image, transformation_matrix,
                        cv::Size(image_input.cols, image_input.rows), cv::INTER_NEAREST + cv::WARP_INVERSE_MAP,
                        cv::BORDER_TRANSPARENT, 80);
#endif
#else
#if INTERPOLATION == 1
    cv::warpAffine(image_input, opencv_image, transformation_matrix, cv::Size(image_input.cols, image_input.rows),
                   cv::INTER_LINEAR + cv::WARP_INVERSE_MAP, cv::BORDER_TRANSPARENT, 80);
#else
    cv::warpAffine(image_input, opencv_image, transformation_matrix, cv::Size(image_input.cols, image_input.rows),
                   cv::INTER_NEAREST + cv::WARP_INVERSE_MAP, cv::BORDER_TRANSPARENT, 80);
#endif
#endif

    cv::imwrite("opencv_output.png", opencv_image);
}


#endif //_XF_OCV_REF_HPP_


/*! \} */
