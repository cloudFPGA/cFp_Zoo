#!/usr/bin/env python

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

## @file   test_median_blur_video_threaded_ray.py
## @author DID
## @date   April 2022
## @brief  A python script for testing the cF median_blur kernel in ray distributed environment. 

'''
Usage:
   test_harris_standalone.py <imagefile name>

   Run harris in opencv 

'''

import sys
import os
import numpy as np
import cv2 as cv
import logging
import time
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
#from pandasgui import show
import math
from scipy.interpolate import griddata



debug_level = logging.DEBUG
logging.basicConfig(stream=sys.stdout, level=debug_level)

REPEATS = 5
kernels = ["MedianFilter", "HarrisDetector", "Histogram", "CannyEdgeDetector", "HoughLineTransform", 
           "Demosaicing", "GammaCorrection", "Sobel", "GaussianBlur", "Laplacian", "Remap", "PyrUp",
           "PyrDown", "Rotate", "WarpAffine", "HOGDescriptor"]

frame = cv.imread(cv.samples.findFile("CARLA.jpg"))
if frame is None:
    sys.exit("Could not read the image.")
else:
    orig_frame = cv.cvtColor(frame, cv.COLOR_RGB2GRAY)



def gammaCorrection(src, gamma):
    invGamma = 1 / gamma
 
    table = [((i / 255) ** invGamma) * 255 for i in range(256)]
    table = np.array(table, np.uint8)
 
    return cv.LUT(src, table)





# When everything done, release the video capture object
toc_capture = time.perf_counter()

#elapsed_times = np.zeros(shape=(REPEATS,len(kernels)), dtype=float)

elapsed_times = [[0 for j in range(REPEATS)] for i in range(len(kernels))]
results = [[0 for j in range(REPEATS)] for i in range(len(kernels))]

print(results)
for i in range(len(kernels)):
    frame = orig_frame.copy()
    for j in range(REPEATS):
        start_time = time.time()
        print(i,j)
        # Opencv function call
        if (i == 0):
            results[i][j] = cv.medianBlur(frame, 9)
            #time.sleep(1)
        elif (i == 1):
            blockSize = 2
            ksize  = 3
            freeParameter = 0.04
            results[i][j] = cv.cornerHarris(frame, blockSize, ksize , freeParameter)
            #result is dilated for marking the corners, not important
            results[i][j] = cv.dilate(results[i][j],None)
            # Threshold for an optimal value, it may vary depending on the image.
            tmp = frame
            tmp[results[i][j]>0.01*results[i][j].max()]=[255]
            results[i][j] = tmp
        elif (i == 2):
            # find frequency of pixels in range 0-255
            channels = [0]
            mask = None
            histSize = [256]
            ranges = [0,256]
            tmp = [frame]
            hist = cv.calcHist(tmp, channels, mask, histSize, ranges)
            results[i][j] = hist
        elif (i == 3):     
            # Setting parameter values
            t_lower = 50  # Lower Threshold
            t_upper = 150  # Upper threshold
            # Applying the Canny Edge filter
            canny = cv.Canny(frame, t_lower, t_upper)
            results[i][j] = canny
        elif (i == 4):                 
            lines = cv.HoughLines(canny, 1, np.pi / 180, 150, None, 0, 0)
        elif (i == 5):            
            results[i][j] = cv.demosaicing(frame, cv.COLOR_BayerRG2BGR_VNG); 
        elif (i == 6):            
            results[i][j] = gammaCorrection(frame, 2.2)
        elif (i == 7):            
            x = cv.Sobel(frame, cv.CV_64F, 1, 0, ksize=5)
            y = cv.Sobel(frame, cv.CV_64F, 0, 1, ksize=5)
            absx= cv.convertScaleAbs(x)
            absy = cv.convertScaleAbs(y)
            results[i][j] = cv.addWeighted(absx, 0.5, absy, 0.5,0)
        elif (i == 8):
            results[i][j] = cv.GaussianBlur(frame,(35,35),cv.BORDER_DEFAULT)
        elif (i == 9):  
            ddepth = cv.CV_16S
            kernel_size = 5       
            results[i][j] = cv.Laplacian(frame, ddepth, ksize=kernel_size)
        elif (i == 10):
            source = np.array([
                [315, 15],
                [962, 18],
                [526, 213],
                [754, 215],
                [516, 434],
                [761, 433],
                [225, 701],
                [1036, 694],
                ], dtype=int)
            destination = np.array([
                [14, 14],
                [770, 14],
                [238, 238],
                [546, 238],
                [238, 546],
                [546, 546],
                [14, 770],
                [770, 770]
                ], dtype=int)
            grid_x, grid_y = np.mgrid[0:783:784j, 0:783:784j]
            grid_z = griddata(destination, source, (grid_x, grid_y), method='cubic')
            map_x = np.append([], [ar[:,1] for ar in grid_z]).reshape(784,784)
            map_y = np.append([], [ar[:,0] for ar in grid_z]).reshape(784,784)
            map_x_32 = map_x.astype('float32')
            map_y_32 = map_y.astype('float32')
            results[i][j] = cv.remap(frame, map_x_32, map_y_32, cv.INTER_CUBIC)
        elif (i == 11):
            rows, cols = map(int, frame.shape)
            results[i][j] = cv.pyrUp(frame, ( cols*2, rows*2 ))
        elif (i == 12):
            rows, cols = map(int, frame.shape)
            results[i][j] = cv.pyrDown(frame, ( cols/2, rows/2 ))
        elif (i == 13):        
            results[i][j] = cv.rotate(frame, cv.ROTATE_90_CLOCKWISE)
        elif (i == 14):            
            rows, cols = frame.shape[:2]
            # Define the 3 pairs of corresponding points 
            input_pts = np.float32([[0,0], [cols-1,0], [0,rows-1]])
            output_pts = np.float32([[cols-1,0], [0,0], [cols-1,rows-1]])
            input_pts = np.float32([[50, 50],
                   [200, 50], 
                   [50, 200]])
            output_pts = np.float32([[10, 100],
                   [200, 50], 
                   [100, 250]])            
            # Calculate the transformation matrix using cv2.getAffineTransform()
            M = cv.getAffineTransform(input_pts , output_pts)
            # Apply the affine transformation using cv2.warpAffine()
            results[i][j] = cv.warpAffine(frame, M, (cols,rows))
        elif (i == 14):            
            hog = cv.HOGDescriptor()
            h = hog.compute(frame)
            results[i][j] = h 
            
        end_time = time.time()
        elapsed_times[i][j] = (end_time - start_time)

for i in range(len(kernels)):
    image_name = "CARLA_out_kernel_" + kernels[i] + ".jpg"
    cv.imwrite(image_name, results[i][0])
    logging.info("First saved image of " + kernels[i] + ": " + image_name)
    
elapsed_times = np.sort(elapsed_times)
average_elapsed_time = sum(elapsed_times) / REPEATS
print(elapsed_times)
print("Time required to submit a trivial function call:")
print("    Average: {}".format(average_elapsed_time))
#print("    90th percentile: {}".format(elapsed_times[round((90/100) * REPEATS)-1]))
#print("    99th percentile: {}".format(elapsed_times[round((99/100) * REPEATS)-1]))
#print("    best:            {}".format(elapsed_times[0]))
#print("    worst:           {}".format(elapsed_times[round((99.9/100) * REPEATS)-1]))

elapsed_times = np.transpose(elapsed_times)
df = pd.DataFrame(elapsed_times)

ax = sns.boxplot(data=df)
ax.set_xticklabels(kernels)
ax.legend(loc='best')
ax.tick_params(axis='x', labelrotation=30)

ax.set_xlabel("Computer Vision Kernel", fontsize = 20)
ax.set_ylabel("Exec. Time (s)", fontsize = 20)
#gui = show(df)

# to show histogram
#plt.plot(hist)

# to show hughlines
lines = cv.HoughLinesP(canny, 1, np.pi/180.0, 120, minLineLength=10, maxLineGap=250)
for line in lines:
    x1, y1, x2, y2 = line[0]
   # cv2.line(img, (x1, y1), (x2, y2), (255, 0, 0), 3)
    cv.line(frame,(x1,y1),(x2,y2),(0,0,255),2)
cv.imwrite('file.png', frame)



plt.show()
