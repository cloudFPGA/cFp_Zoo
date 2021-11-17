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
 * @brief      The Sobel reference functions used in TB
 * @author     DCO
 * @date       Nov 2021
 * 
 * @ingroup SobelTB
 * @addtogroup SobelTB
 * \{
 *****************************************************************************/


#ifndef _XF_OCV_REF_HPP_
#define _XF_OCV_REF_HPP_
#include "xf_config_params.h"

using namespace cv;
using namespace std;


void ocv_ref(cv::Mat img_gray, cv::Mat& ocv_out_img, int ksize) {

    cv::medianBlur (img_gray, ocv_out_img, ksize);

}


#endif


/*! \} */
