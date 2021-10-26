## @file   test_median_blur_numpi.py
## @author DID
## @date   November 2020
## @brief  A python script for testing the cF median_blur kernel. The script takes as argument the fpga 
##         ip, the port and the numpi array of an image to be processed. This array should be an 1-D 
##         array, containing all pixels of a CV MAT in CV_8UC1. The kernel will rerurn a numpi
##         array which is the array with only the detected points.

import sys
import os
import numpy as np
import cv2

trieres_lib=os.environ['cFpRootDir'] + "HOST/vision/median_blur/languages/python/build"
sys.path.append(trieres_lib)

import _trieres_median_blur_numpi

# size of image to be processed on fpga (the bitstream should be already fixed to this)
height = width = 256

num_frame = 1

# path  
image_in_filename = os.environ['cFpRootDir'] + "ROLE/vision/hls/median_blur/test/512x512.png"
image_out_filename = image_in_filename + "_fpga_points_out_frame_" + str(num_frame) + ".png"

for i in range(10):
    # Reading an image in unchanged mode 
    image = cv2.imread(image_in_filename, cv2.IMREAD_UNCHANGED) 

    # Converting to grayscale
    image = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)

    # Adjusting the image file if needed
    if ((image.shape[0] != height) or (image.shape[1] != width)):
        print("WARNING: The image was resized from [", image.shape[0] , " x ", image.shape[1] , "] to [", height  , " x ", width, "]")    
        dim = (width, height) 
        image = cv2.resize(image, dim, interpolation = cv2.INTER_LINEAR) 

    # Flattening the image from 2D to 1D
    image = image.flatten()

    total_size = height * width

    input_array = image

#for i in range(100):

    output_array = _trieres_median_blur_numpi.median_blur(input_array, total_size, "10.12.200.234", "2719")

    # Convert 1D array to a 2D numpy array of 2 rows and 3 columns
    output_array_2d = np.reshape(output_array, (height, width))

cv2.imwrite(image_out_filename, output_array_2d)
print("INFO: the output file is saved at : " + image_out_filename)
