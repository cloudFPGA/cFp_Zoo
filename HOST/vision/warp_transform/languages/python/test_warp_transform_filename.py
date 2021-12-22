# *****************************************************************************
# *                            cloudFPGA
# *                Copyright 2016 -- 2022 IBM Corporation
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *     http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *----------------------------------------------------------------------------

## @file   test_warp_transform_filename.py
## @author DID
## @date   November 2020
## @brief  A python script for testing the cF warp_transform kernel. The script takes as argument the fpga 
##         ip, the port and the full path name of an image file to be processed. This path name is
##         given as a python string. The kernel will rerurn two full path names for the two returned
##         images, i.e. an image with only the detected points and the original image anottated with
##         the detected points.

import sys
import os
import numpy as np
trieres_lib=os.environ['cFpRootDir'] + "HOST/vision/warp_transform/languages/python/build"
sys.path.append(trieres_lib)

import _trieres_warp_transform_filename

input = os.environ['cFpRootDir'] + "ROLE/vision/hls/warp_transform/test/512x512.png"

out, output_image, output_points = _trieres_warp_transform_filename.warp_transform("localhost", "1234", input)

print("The output image file is " + output_image)

print("The output points file is " + output_points)
