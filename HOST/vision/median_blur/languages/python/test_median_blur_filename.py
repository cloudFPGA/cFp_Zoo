## @file   test_median_blur_filename.py
## @author DID
## @date   November 2020
## @brief  A python script for testing the cF median_blur kernel. The script takes as argument the fpga 
##         ip, the port and the full path name of an image file to be processed. This path name is
##         given as a python string. The kernel will rerurn two full path names for the two returned
##         images, i.e. an image with only the detected points and the original image anottated with
##         the detected points.

import sys
import os
import numpy as np
trieres_lib=os.environ['cFpRootDir'] + "HOST/vision/median_blur/languages/python/build"
sys.path.append(trieres_lib)

import _trieres_median_blur_filename

input = os.environ['cFpRootDir'] + "ROLE/vision/hls/median_blur/test/512x512.png"

out, output_image, output_points = _trieres_median_blur_filename.median_blur("localhost", "1234", input)

print("The output image file is " + output_image)

print("The output points file is " + output_points)
