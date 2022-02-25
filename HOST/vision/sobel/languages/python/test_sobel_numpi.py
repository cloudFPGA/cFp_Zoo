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

## @file   test_sobel_numpi.py
## @author DID
## @date   November 2020
## @brief  A python script for testing the cF sobel kernel. The script takes as argument the fpga 
##         ip, the port and the numpi array of an image to be processed. This array should be an 1-D 
##         array, containing all pixels of a CV MAT in CV_8UC1. The kernel will rerurn a numpi
##         array which is the array with only the detected points.

import sys
import os
import numpy as np
import cv2

trieres_lib=os.environ['cFpRootDir'] + "HOST/vision/sobel/languages/python/build"
sys.path.append(trieres_lib)

import _trieres_sobel_numpi

# size of image to be processed on fpga (the bitstream should be already fixed to this)
height = width = 256

num_frame = 1

# path  
image_in_filename = os.environ['cFpRootDir'] + "ROLE/vision/hls/sobel/test/512x512.png"
image_out_filename = image_in_filename + "_fpga_points_out_frame_" + str(num_frame) + ".png"

ROI = False


def crop_square(img, size, interpolation=cv2.INTER_AREA):
    h, w = img.shape[:2]
    print("h="+str(h))
    print("w="+str(w))
    min_size = np.amin([h,w])
    print("min_size="+str(min_size))

    # Centralize and crop
    crop_img = img[int(h/2-min_size/2):int(h/2+min_size/2), int(w/2-min_size/2):int(w/2+min_size/2)]
    y1=10
    y2=y1+100
    x1=10
    x2=x1+100
    roi = crop_img[y1:y2, x1:x2]
    resized = cv2.resize(roi , (size, size), interpolation=interpolation)

    return resized


def patch_square(crop_img, img, interpolation=cv2.INTER_AREA):
    h, w = img.shape[:2]
    print("h="+str(h))
    print("w="+str(w))
    min_size = np.amin([h,w])
    print("min_size="+str(min_size))

    # Centralize and patch
    img[int(h/2-min_size/2):int(h/2+min_size/2), int(w/2-min_size/2):int(w/2+min_size/2)] = crop_img

    return img






for i in range(1):
    # Reading an image in unchanged mode 
    image = cv2.imread(image_in_filename, cv2.IMREAD_UNCHANGED) 

    # Converting to grayscale
    image = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)

    # Adjusting the image file if needed
    if ((image.shape[0] != height) or (image.shape[1] != width)):
    	if ROI:
	        print("WARNING: An image of size [", height  , " x ", width, "] will be cropped from input image of size [", image.shape[0] , " x ", image.shape[1] , "]")    
	        image_big = image
	        image = crop_square(image_big, width, interpolation = cv2.INTER_AREA)		
    	else:
	        print("WARNING: The image was resized from [", image.shape[0] , " x ", image.shape[1] , "] to [", height  , " x ", width, "]")    
	        dim = (width, height) 
        	image = cv2.resize(image, dim, interpolation = cv2.INTER_LINEAR) 

    # Flattening the image from 2D to 1D
    image = image.flatten()

    total_size = height * width

    input_array = image

#for i in range(100):

    output_array = _trieres_sobel_numpi.sobel(input_array, total_size, "10.12.200.122", "2718")

    # Convert 1D array to a 2D numpy array of 2 rows and 3 columns
    output_array_2d = np.reshape(output_array, (height, width))
    
    #if ROI:
    #    output_array_2d = patch_square(output_array_2d, image_big, interpolation = cv2.INTER_AREA)

cv2.imwrite(image_out_filename, output_array_2d)
print("INFO: the output file is saved at : " + image_out_filename)
