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

debug_level = logging.DEBUG
logging.basicConfig(stream=sys.stdout, level=debug_level)

REPEATS = 10

frames=[]
frame = cv.imread(cv.samples.findFile("CARLA.jpg"))
if frame is None:
    sys.exit("Could not read the image.")
else:
    frame = cv.cvtColor(frame, cv.COLOR_RGB2GRAY)
    for i in range(REPEATS):
        frames.append(frame)
    results = frames
# When everything done, release the video capture object
toc_capture = time.perf_counter()

elapsed_times = []
for _ in range(REPEATS):
    start_time = time.time()
    
    # Opencv function call
    results[i] = cv.medianBlur(frames[i], 9)
    
    end_time = time.time()
    elapsed_times.append(end_time - start_time)
    
image_name = "CARLA_out_"+str(0)+".jpg"
cv.imwrite(image_name, results[0])
logging.info("First saved image: " + image_name)
    
elapsed_times = np.sort(elapsed_times)
average_elapsed_time = sum(elapsed_times) / REPEATS
print(elapsed_times)
print("Time required to submit a trivial function call:")
print("    Average: {}".format(average_elapsed_time))
print("    90th percentile: {}".format(elapsed_times[round((90/100) * REPEATS)-1]))
print("    99th percentile: {}".format(elapsed_times[round((99/100) * REPEATS)-1]))
print("    best:            {}".format(elapsed_times[0]))
print("    worst:           {}".format(elapsed_times[round((99.9/100) * REPEATS)-1]))

