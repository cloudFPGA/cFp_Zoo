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
   test_median_blur_numpi_video_threaded.py <video file name>

   Shows how python ray capabilities can be used
   to organize parallel frame processing pipeline
   with cloudFPGA.

'''

import ray
from ray.util.queue import Queue

import sys
import os
import numpy as np
import cv2 as cv

ROI = True
accel_mode = True

config_file=os.environ['cFpRootDir'] + "HOST/vision/median_blur/languages/cplusplus/include/config.h"

with open(config_file) as cfg:
    for line in cfg:
        if "#define FRAME_WIDTH" in line:
            width = int(line.split()[2])
        elif "#define FRAME_HEIGHT" in line:
            height = int(line.split()[2])
try:
   print("Found in " + config_file + ": width = "+str(width) + ", height = "+str(height))
   total_size = height * width
except:
    print("Coudln't find FRAME_WIDTH or FRAME_HEIGHT in "+ config_file + ". Aborting...")
    exit(0)

def crop_square_roi(img, size, interpolation=cv.INTER_AREA):
    h, w = img.shape[:2]
    if ROI:
        if (h>height) and (w>width):
            roi_x_pos = int((w-width) /2)
            roi_y_pos = int((h-height)/2)
            crop_img = img[int(roi_y_pos):int(roi_y_pos+height), int(roi_x_pos):int(roi_x_pos+width)]
        else:
            crop_img = img
            print("WARNING: The input image of [", h , " x ", w , "] is not bigger to crop a ROI of [", height  , " x ", width, "]. Will just resize")
    else:
        min_size = np.amin([np.amin([h,w]), np.amin([height,width])])
        # Centralize and crop
        crop_img = img[int(h/2-min_size/2):int(h/2+min_size/2), int(w/2-min_size/2):int(w/2+min_size/2)]
    
    # Adjusting the image file if needed
    if ((crop_img.shape[0] != height) or (crop_img.shape[1] != width)):
        print("WARNING: The image was resized from [", crop_img.shape[0] , " x ", crop_img.shape[1] , "] to [", height  , " x ", width, "]")
        resized = cv.resize(crop_img , (size, size), interpolation=interpolation)
    else:
        resized = crop_img
    return resized

def patch_sqaure_roi(orig, frame, interpolation=cv.INTER_AREA):
    h_orig,  w_orig  = orig.shape[:2]
    h_frame, w_frame = frame.shape[:2]
      
    patched_img = orig.copy()
        
    if (h_orig>h_frame) and (w_orig>w_frame):
        roi_x_pos = int((w_orig-w_frame)/2)
        roi_y_pos = int((h_orig-h_frame)/2)
        frame_backtorgb = cv.cvtColor(np.float32(frame),cv.COLOR_GRAY2RGB)
        patched_img[int(roi_y_pos):int(roi_y_pos+h_frame), int(roi_x_pos):int(roi_x_pos+w_frame),:] = frame_backtorgb
    else:
        patched_img = frame
        print("WARNING: The input image of [", h_orig , " x ", w_orig , "] is not bigger to embed a ROI of [", h_frame  , " x ", w_frame, "]. Will just resize")
    # Adjusting the image file if needed
    if ((patched_img.shape[0] != h_orig) or (patched_img.shape[1] != w_orig)):
        print("WARNING: The image was resized from [", patched_img.shape[0] , " x ", patched_img.shape[1] , "] to [", h_orig  , " x ", w_orig, "]")
        resized = cv.resize(patched_img , (w_orig, h_orig), interpolation=interpolation)
    else:
        resized = patched_img
    return resized


ray.init(dashboard_port=50051, num_cpus=8)


# You can pass this object around to different tasks/actors
fpgas_queue = Queue(maxsize=100)

@ray.remote
def consumer(accel_mode, fpgas_queue, frame):
    orig = frame
    frame_ret = cv.cvtColor(frame, cv.COLOR_RGB2GRAY)    
    frame_ret = crop_square_roi(frame_ret, width, interpolation = cv.INTER_AREA)    
    if accel_mode:
        next_item = fpgas_queue.get(block=True, timeout=100)
        print(f"will work on {next_item} and then put in back in the fpgas_queue")
        # Adjusting the image file if needed
        frame_ret = cv.medianBlur(frame_ret, 9)
        fpgas_queue.put(next_item)
        print(f"finished working on {next_item} Now it is back in the fpgas_queue")
    else:
        frame_ret = cv.medianBlur(frame_ret, 9)        
    if ROI:
        frame_ret = patch_sqaure_roi(orig, frame_ret, cv.INTER_AREA)    
    return frame_ret


try:
    fn = sys.argv[1]
except:
    fn = 0

cap = cv.VideoCapture(fn)
frames = []
frames_ret = []

# Check if camera opened successfully
if (cap.isOpened()== False):
  print("Error opening video stream or file")

# Read until video is completed 
# TODO: find a more efficient way, without loading all video in ram, e.g. PILs
while(cap.isOpened()):
  # Capture frame-by-frame
  ret, frame = cap.read()
  if ret == True:
    frames.append(frame)
  # Break the loop
  else: 
    break

# When everything done, release the video capture object
cap.release()

consumers = [consumer.remote(accel_mode, fpgas_queue, frames[i]) for i in range(len(frames))]

[fpgas_queue.put(j) for j in ([ ["10.12.200.73" , "2718"],
                                ["10.12.200.24" , "2719"],
                                ["10.12.200.11" , "2720"],
                                ["10.12.200.19" , "2721"],
                                ["10.12.200.29" , "2722"]   ])]

results = ray.get(consumers)
print('Tasks executed')

video_name = str(fn)+"_out.avi"
video_out = cv.VideoWriter(video_name, cv.VideoWriter_fourcc('M','J','P','G'), 30, (results[0].shape[1],results[0].shape[0]))
for t in range(len(results)):
    video_out.write(results[t])
video_out.release()
print("Saved video: " + video_name)
