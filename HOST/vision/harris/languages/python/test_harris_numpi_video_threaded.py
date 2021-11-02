#!/usr/bin/env python

## @file   test_harris_numpi_video_threaded.py
## @author DID
## @date   October 2021
## @brief  A python script for testing the cF harris kernel in multi-threaded environment. 

'''
Harris multithreaded video processing sample.
Usage:
   test_harris_numpi_video_threaded.py {<video device number>|<video file name>}

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
video_common_lib=os.environ['cFpRootDir'] + "HOST/vision/harris/languages/python/var"
sys.path.append(video_common_lib)

import numpy as np
import cv2 as cv

from multiprocessing.pool import ThreadPool
from collections import deque

from common import clock, draw_str, StatValue
import video

trieres_lib=os.environ['cFpRootDir'] + "HOST/vision/harris/languages/python/build"
sys.path.append(trieres_lib)

import _trieres_harris_numpi

# size of image to be processed on fpga (the bitstream should be already fixed to this)
height = width = 512
total_size = height * width

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

    try:
        fn = sys.argv[1]
    except:
        fn = 0
    cap = video.create_capture(fn)
    fps = FPS().start()

    video_name = str(fn)+"_out.avi"
    video_out = cv.VideoWriter(video_name, cv.VideoWriter_fourcc('M','J','P','G'), 10, (width,height))
    
    fpgas = deque([ ["10.12.200.3"   , "2718"],
                    ["10.12.200.165" , "2719"]])

    
    def process_frame(frame, t0, accel_mode, fpga):
        if accel_mode:
            print("Will execute on fpga with ip:port: "+fpga[0]+":"+fpga[1])
            # some intensive computation...
            # frame = cv.medianBlur(frame, 19)
            # Converting to grayscale
            frame = cv.cvtColor(frame, cv.COLOR_RGB2GRAY)
            # Adjusting the image file if needed
            if ((frame.shape[0] != height) or (frame.shape[1] != width)):
                print("WARNING: The image was resized from [", frame.shape[0] , " x ", frame.shape[1] , "] to [", height  , " x ", width, "]")
            dim = (width, height)
            frame = cv.resize(frame, dim, interpolation = cv.INTER_LINEAR)
            # Flattening the image from 2D to 1D
            image = frame.flatten()
            output_array = _trieres_harris_numpi.harris(image, total_size, fpga[0], fpga[1])
            # Convert 1D array to a 2D numpy array 
            frame = np.reshape(output_array, (height, width))
            print("Declare free the fpga: "+str(fpga))
            fpgas.appendleft(fpga)
        else:
            frame = cv.medianBlur(frame, 19)
            frame = cv.medianBlur(frame, 19)
            frame = cv.medianBlur(frame, 19)
        return frame, t0

    threadn = 4 #cv.getNumberOfCPUs()
    pool = ThreadPool(processes = threadn)
    pending = deque()

    threaded_mode = True
    accel_mode = True
    latency = StatValue()
    frame_interval = StatValue()
    last_frame_time = clock()
    while True:
        while len(pending) > 0 and pending[0].ready() and len(fpgas) > 0:
            res, t0 = pending.popleft().get()
            latency.update(clock() - t0)
#            video_out.write(res)
            draw_str(res, (20, 20), "threaded       :  " + str(threaded_mode))
            draw_str(res, (20, 40), "cloudFPA       :  " + str(accel_mode))
            draw_str(res, (20, 60), "latency        :  %.1f ms" % (latency.value*1000))
            draw_str(res, (20, 80), "frame interval :  %.1f ms" % (frame_interval.value*1000))
            draw_str(res, (20, 100), "FPS           :  %.1f" % (1.0/frame_interval.value))
            #cv.imshow('threaded video', res)
        if len(pending) < threadn and len(fpgas) != 0:
            _ret, frame = cap.read()
            if _ret is False:
                print("Reached EOF.")
                print("Saved video: " + video_name)
                video_out.release()
                break
            else:
                video_out.write(frame)
            t = clock()
            frame_interval.update(t - last_frame_time)
            last_frame_time = t
            # update the FPS counter
            fps.update()            
            if accel_mode:
                fpga = fpgas.popleft()
            else:
                fpga = 0
            if threaded_mode:
                task = pool.apply_async(process_frame, (frame.copy(), t, accel_mode, fpga))
            else:
                task = DummyTask(process_frame(frame, t, accel_mode, fpga))
            pending.append(task)
        else:
            if accel_mode:
                print("Waiting for a free fpga")
            
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

