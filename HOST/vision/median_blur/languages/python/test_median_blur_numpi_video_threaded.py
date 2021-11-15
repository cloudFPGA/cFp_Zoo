#!/usr/bin/env python

## @file   test_median_blur_numpi_video_threaded.py
## @author DID
## @date   October 2021
## @brief  A python script for testing the cF median_blur kernel in multi-threaded environment. 

'''
MedianBlur multithreaded video processing sample.
Usage:
   test_median_blur_numpi_video_threaded.py {<video device number>|<video file name>}

   Shows how python threading capabilities can be used
   to organize parallel captured frame processing pipeline
   for smoother playback.

Keyboard shortcuts:

   ESC - exit
   f - switch between CPU and cloudFPGA version (pre-programming is required)
   space - switch between multi and single threaded processing
'''

# Python 2/3 compatibility
from __future__ import print_function

import sys
import os
video_common_lib=os.environ['cFpRootDir'] + "HOST/vision/common/languages/python/var"
sys.path.append(video_common_lib)

import numpy as np
import cv2 as cv

import multiprocessing

from multiprocessing.pool import ThreadPool
from collections import deque

# Manager to create shared object.
manager = multiprocessing.Manager()

from common import clock, draw_str, StatValue
import video
import time 

trieres_lib=os.environ['cFpRootDir'] + "HOST/vision/median_blur/languages/python/build"
sys.path.append(trieres_lib)

import _trieres_median_blur_numpi

ROI = True

# import the necessary packages
import datetime
class FPS:
	def __init__(self):
		# store the start time, end time, and total number of frames
		# that were examined between the start and end intervals
		self._start = None
		self._end = None
		self._numFrames = 0
	def start(self):
		# start the timer
		self._start = datetime.datetime.now()
		return self
	def stop(self):
		# stop the timer
		self._end = datetime.datetime.now()
	def update(self):
		# increment the total number of frames examined during the
		# start and end intervals
		self._numFrames += 1
	def elapsed(self):
		# return the total number of seconds between the start and
		# end interval
		return (self._end - self._start).total_seconds()
	def fps(self):
		# compute the (approximate) frames per second
		return self._numFrames / self.elapsed()




class DummyTask:
    def __init__(self, data):
        self.data = data
    def ready(self):
        return True
    def get(self):
        return self.data

def main():
    import sys

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
        
    try:
        fn = sys.argv[1]
    except:
        fn = 0
    cap = video.create_capture(fn)
    fps = FPS().start()

    # Create a lock.
    lock = manager.Lock()

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
        
        patched_img = orig
        
        if (h_orig>h_frame) and (w_orig>w_frame):
                roi_x_pos = int((w_orig-w_frame)/2)
                roi_y_pos = int((h_orig-h_frame)/2)
                frame_backtorgb = cv.cvtColor(frame,cv.COLOR_GRAY2RGB)
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


    
    def process_frame(frame, t0, threaded_mode, accel_mode, fpga, fpgas):
        # Converting to grayscale
        orig = frame
        frame = cv.cvtColor(frame, cv.COLOR_RGB2GRAY)

        # Adjusting the image file if needed
        ##frame = cv.resize(frame, (width, height), interpolation = cv.INTER_LINEAR)
        frame = crop_square_roi(frame, width, interpolation = cv.INTER_AREA)
        
        if accel_mode:
            #print("Will execute on fpga with ip:port: "+fpga[0]+":"+fpga[1])
            # some intensive computation...
            # frame = cv.medianBlur(frame, 19)
            # Flattening the image from 2D to 1D
            image = frame.flatten()
            output_array = _trieres_median_blur_numpi.median_blur(image, total_size, fpga[0], fpga[1])
            # Convert 1D array to a 2D numpy array 
            #time.sleep(1)
            frame = np.reshape(output_array, (height, width))
            #time.sleep(1)
            #print("Declare free the fpga: "+str(fpga))
            lock.acquire()
            if threaded_mode:
                fpgas.append(fpga)
            else:
                fpgas.appendleft(fpga)
            lock.release()
        else:
            #frame = cv.medianBlur(frame, 9)
            #time.sleep(10)
            frame = cv.medianBlur(frame, 9)
        if ROI:
               frame = patch_sqaure_roi(orig, frame, cv.INTER_AREA)
        #print(frame.shape)
        #exit(0)
        return frame, t0

    
    threaded_mode = False
    accel_mode = True

    fpgas = deque([["10.12.200.195" , "2718"],
                   ["10.12.200.216" , "2719"],
                   ["10.12.200.171" , "2720"],
                   ["10.12.200.19" , "2721"],
                   ["10.12.200.29" , "2722"]])

    if accel_mode:
        threadn = len(fpgas) 
    else:
        threadn = cv.getNumberOfCPUs()
    pool = ThreadPool(processes = threadn)
    pending = deque()

    latency = StatValue()
    frame_interval = StatValue()
    last_frame_time = clock()
    while True:
        fpga = 0
        #print("len(pending)="+str(len(pending)))
        while len(pending) > 0 and pending[0].ready() :
            #print("Before pending.popleft().get()")
            res, t0 = pending.popleft().get()
            #print(type(fpga))
            #print(str(fpga))
            #exit(0)
            latency.update(clock() - t0)
            draw_str(res, (20, 20), "threaded       :  " + str(threaded_mode))
            draw_str(res, (20, 40), "cloudFPA       :  " + str(accel_mode))
            draw_str(res, (20, 60), "latency        :  %.1f ms" % (latency.value*1000))
            draw_str(res, (20, 80), "frame interval :  %.1f ms" % (frame_interval.value*1000))
            draw_str(res, (20, 100), "FPS           :  %.1f" % (1.0/frame_interval.value))
            try:
                video_out.write(res)
            except:
                video_name = str(fn)+"_out.avi"
                video_out = cv.VideoWriter(video_name, cv.VideoWriter_fourcc('M','J','P','G'), 30, (res.shape[1],res.shape[0]))
                #print("video_out Size is:"+str(res.shape[1])+","+str(res.shape[0]))
            #cv.imshow('threaded video', res)
        if len(pending) < threadn: # and len(fpgas) != 0:
            _ret, frame = cap.read()
            if _ret is False:
                print("Reached EOF.")
                print("Saved video: " + video_name)
                video_out.release()
                break
            #print("frame Size is:"+str(frame.shape[1])+","+str(frame.shape[0]))
            t = clock()
            frame_interval.update(t - last_frame_time)
            last_frame_time = t
            # update the FPS counter
            fps.update()            
            if accel_mode:
                lock.acquire()
                fpga = fpgas.popleft()
                #print("Reserved the fpga:"+str(fpga))
                lock.release()
            else:
                fpga = 0
            if threaded_mode:
                task = pool.apply_async(process_frame, (frame.copy(), t, threaded_mode, accel_mode, fpga, fpgas))
                fpga = 0
            else:
                task = DummyTask(process_frame(frame, t, threaded_mode, accel_mode, fpga, fpgas))
            pending.append(task)
        else:
            if accel_mode:
                print("Waiting for a free fpga")
            else:
                print("Waiting for a free thread")
        #if accel_mode and type(fpga) is list:
        #    print("Declare free the fpga: "+str(fpga))
        #    fpgas.appendleft(fpga)
                
        ch = cv.waitKey(1)
        if ch == ord(' '):
            threaded_mode = not threaded_mode
        if ch == ord('f'):
            accel_mode = not accel_mode
        if ch == 27:
            break


    print('Done')
 
    # stop the timer and display FPS information
    fps.stop()
    print("[INFO] elasped time: {:.2f}".format(fps.elapsed()))
    print("[INFO] approx. FPS: {:.2f}".format(fps.fps()))

if __name__ == '__main__':
    print(__doc__)
    main()
    #cv.destroyAllWindows()

