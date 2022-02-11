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

/*****************************************************************************
 * @file       cv_warp_transform_config.hpp
 * @brief      The configuration of a WarpTransform Example application (UDP or TCP)
 * @author     DCO
 * @date       June 2022
 *----------------------------------------------------------------------------
 *
 * @details    This file contains definitios used both in HLS and host code of 
 *             cFp Vitis WarpTransform application example.
 *
 *----------------------------------------------------------------------------
 * 
 * @ingroup WarpTransform
 * @addtogroup WarpTransform
 * \{
 *****************************************************************************/

#ifndef _CV_WARP_TRANSFORM_HPP_
#define _CV_WARP_TRANSFORM_HPP_
// transform type 0-NN 1-BILINEAR
#define INTERPOLATION 0

// transform type 0-AFFINE 1-PERSPECTIVE
#define TRANSFORM_TYPE 0

#if TRANSFORM_TYPE == 1
#define TRMAT_DIM2 3
#define TRMAT_DIM1 3
#else
#define TRMAT_DIM2 3
#define TRMAT_DIM1 2
#endif

#include "opencv2/opencv.hpp"
#include "../../../../../../ROLE/vision/hls/warp_transform/include/xf_ocv_ref.hpp"  // For SW reference WarpTransform from OpenCV
#include "../../../../../../ROLE/vision/hls/warp_transform/include/warp_transform_network_config.hpp"  // For CMD commands

#endif //_CV_WARP_TRANSFORM_HPP_