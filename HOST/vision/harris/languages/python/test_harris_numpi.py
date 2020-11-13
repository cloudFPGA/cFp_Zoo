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
import cv2
import matplotlib.pyplot as plt
from PIL import Image

trieres_lib=os.environ['cFpRootDir'] + "HOST/vision/harris/languages/python/build"
sys.path.append(trieres_lib)

import _trieres_harris_numpi

# size of image to be processed on fpga (the bitstream should be already fixed to this)
height = width = 256

num_frame = 1

# path  
image_in_filename = os.environ['cFpRootDir'] + "ROLE/vision/hls/harris/test/512x512.png"
image_out_filename = image_in_filename + "_fpga_points_out_frame_" + str(num_frame) + ".png"

# Reading an image in unchanged mode 
image = cv2.imread(image_in_filename, cv2.IMREAD_UNCHANGED) 

# Converting to grayscale
image = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)

# Adjusting the image file if needed
if ((image.shape[0] != height) or (image.shape[1] != width)):
  print("Warning: The image was resized from [", image.shape[0] , " x ", image.shape[1] , "] to [", height  , " x ", width, "]")
  dim = (width, height) 
  image = cv2.resize(image, dim, interpolation = cv2.INTER_LINEAR) 

# Flattening the image from 2D to 1D
image = image.flatten()

total_size = height * width

input_array = image

output_array = _trieres_harris_numpi.harris(input_array, total_size, "localhost", "1234")

# Convert 1D array to a 2D numpy array of 2 rows and 3 columns
output_array_2d = np.reshape(output_array, (height, width))

cv2.imwrite(image_out_filename, output_array_2d)
print("INFO: the output file is saved at : " + image_out_filename)

