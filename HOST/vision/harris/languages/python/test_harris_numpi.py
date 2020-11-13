## @file   test_harris_numpi.py
## @author DID
## @date   November 2020
## @brief  A python script for testing the cF harris kernel. The script takes as argument the fpga 
##         ip, the port and the numpi array of an image to be processed. This array should be an 1-D 
##         array, containing all pixels of a CV MAT in CV_8UC1. The kernel will rerurn a numpi
##         array whoch is the array with only the detected points.

import sys
import os
import numpy as np
trieres_lib=os.environ['cFpRootDir'] + "HOST/vision/harris/languages/python/build"
sys.path.append(trieres_lib)

import _trieres_harris_numpi

input = os.environ['cFpRootDir'] + "ROLE/vision/hls/harris/test/512x512.png"

total_size = 256 * 256

out = np.array([1.0,2.0]);

out = _trieres_harris_numpi.harris(total_size, "localhost", "1234")



