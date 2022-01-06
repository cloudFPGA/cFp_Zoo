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
 * @file       xf_config_params.h
 * @brief      The WarpTransform IP configuration header
 * @author     DCO
 * @date       Nov 2021
 * 
 * @ingroup WarpTransformHLS
 * @addtogroup WarpTransformHLS
 * \{
 *****************************************************************************/
#ifndef _XF_WARPTRANSFORM_CONFIG_PARAMS_H_
#define _XF_WARPTRANSFORM_CONFIG_PARAMS_H_


#define RO 0 // Resource Optimized (8-pixel implementation)
#define NO 1 // Normal Operation (1-pixel implementation)

#define RGBA 0
#define GRAY 1
// // Number of rows in the input image
// #define HEIGHT 2160
// // Number of columns in  in the input image
// #define WIDTH 3840

// // Number of rows of input image to be stored
// #define NUM_STORE_ROWS 100

// // Number of rows of input image after which output image processing must start
// #define START_PROC 50
// // transform type 0-NN 1-BILINEAR
// #define INTERPOLATION 0

// // transform type 0-AFFINE 1-PERSPECTIVE
// #define TRANSFORM_TYPE 0
// #define XF_USE_URAM false

#endif // _XF_WARPTRANSFORM_CONFIG_PARAMS_H_
/*! \} */
