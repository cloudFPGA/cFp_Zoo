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
import logging
import time
from trieres import *

accel_mode = False
debug_level = logging.DEBUG
logging.basicConfig(stream=sys.stdout, level=debug_level)

width  = 3840
height = 2160
total_size = height * width

DATASET_SIZE = 50
split = 1
REPEATS = 1

#ray.init(dashboard_port=50051, num_cpus=12)
#ray.init(address='ray://192.168.1.8:10001')
ray.init(address='ray://10.1.1.24:10001')

#print('''This cluster consists of
#    {} nodes in total
#    {} CPU resources in total
#'''.format(len(ray.nodes()), ray.cluster_resources()['CPU']))

# You can pass this object around to different tasks/actors
fpgas_queue = Queue(maxsize=100)

#@ray.remote
def consumer(accel_mode, fpgas_queue, frames, i, debug_level=debug_level):
    logging.basicConfig(stream=sys.stdout, level=debug_level)
    
    frame = frames[i]
    if accel_mode:
        next_item = fpgas_queue.get(block=True, timeout=100)
        logging.debug(f"will work on {next_item} and then put in back in the fpgas_queue")
        # Flattening the image from 2D to 1D
        image = frame.flatten()        
        output_array = trieres.vision.median_blur(image, total_size, next_item[0], int(next_item[1]), debug_level=debug_level)
        frame = np.reshape(output_array, (height, width))
        #frame = cv.medianBlur(frame, 9)
        fpgas_queue.put(next_item)
        logging.debug(f"finished working on {next_item} Now it is back in the fpgas_queue")
    else:
        logging.debug(f"Apllying medianBlur")
        frame = cv.medianBlur(frame, 9)
    return frame


@ray.remote
def mega_work(accel_mode, fpgas_queue, frames, debug_level, start, end):
    print("will work on ["+str(start)+"-"+str(end)+"]")
    return [consumer(accel_mode, fpgas_queue, frames, x, debug_level=debug_level) for x in range(start, end)]

def flatten(input):
    if(type(input[0])==list):
        new_list = []
        for i in input:
            for j in i:
                new_list.append(j)
        return new_list
    else:
        return input

try:
    fn = sys.argv[1]
except:
    fn = 0

tic_capture = time.perf_counter()

frames = []
frames_ret = []
frame = cv.imread(cv.samples.findFile("CARLA.jpg"))
if frame is None:
    sys.exit("Could not read the image.")
else:
    frame = cv.cvtColor(frame, cv.COLOR_RGB2GRAY)
    for i in range(DATASET_SIZE):
        frames.append(frame)
    
# When everything done, release the video capture object
toc_capture = time.perf_counter()

elapsed_times = []
for _ in range(REPEATS):
    start_time = time.time()

    tic_consumers = time.perf_counter()
    frames_id = ray.put(frames)
    fpgas_queue_id = ray.put(fpgas_queue)
    
    #consumers = [consumer.remote(accel_mode, fpgas_queue_id, frames_id, i, debug_level=debug_level) for i in range(len(frames))]
    consumers = [mega_work.remote(accel_mode, fpgas_queue_id, frames_id, debug_level, i*split, (i+1)*split) for i in range(int(len(frames)/split))]

    # 256
    #[fpgas_queue.put(j) for j in ([ ["10.12.200.171" , "2718"],   #])]
    #                                ["10.12.200.73"  , "2718"],   # ])]
    #                                ["10.12.200.205" , "2718"],   #])]
    #                                ["10.12.200.69"  , "2718"],   #])]
    #                                ["10.12.200.181" , "2718"]   ])]

    # 512
    [fpgas_queue.put(j) for j in ([ ["10.12.200.9"   , "2718"],   #])]
                                ["10.12.200.212" , "2718"],   #])]
                                ["10.12.200.170" , "2718"],   #])]
                                ["10.12.200.83"  , "2718"],   #])]
                                ["10.12.200.234" , "2718"],   #])]
                                ["10.12.200.219" , "2718"],   #])]
                                ["10.12.200.21"  , "2718"],   #])]
                                ["10.12.200.243" , "2718"],   #])]
                                ["10.12.200.76"  , "2718"],   #])]
                                ["10.12.200.249" , "2718"],   #])]
                                ["10.12.200.140" , "2718"],   #])]
                                ["10.12.200.126" , "2718"],   ])]

    toc_consumers = time.perf_counter()

    tic_exec = time.perf_counter()
    
    results = ray.get(consumers)
    results = flatten(results)

    toc_exec = time.perf_counter()
    logging.info(f"Tasks executed")

    logging.info(f"Tasks executed : {toc_exec - tic_exec:0.4f} seconds")
    logging.info(f"Consumers time : {toc_consumers - tic_consumers:0.4f} seconds")
    logging.info(f"Loading frames : {toc_capture - tic_capture:0.4f} seconds")

    end_time = time.time()
    elapsed_times.append(end_time - start_time)

tic_save = time.perf_counter()
for t in range(len(results)):
    image_name = "CARLA_out_"+str(t)+".jpg"
    cv.imwrite(image_name, results[t])
logging.info("Last saved image: " + image_name)
toc_save = time.perf_counter()
logging.info(f"Saving images  : {toc_save - tic_save:0.4f} seconds")

elapsed_times = np.sort(elapsed_times)
average_elapsed_time = sum(elapsed_times) / REPEATS
print(elapsed_times)
print("Time required to submit a trivial function call:")
print("    Average: {}".format(average_elapsed_time))
print("    90th percentile: {}".format(elapsed_times[round((90/100) * REPEATS)-1]))
print("    99th percentile: {}".format(elapsed_times[round((99/100) * REPEATS)-1]))
print("    best:            {}".format(elapsed_times[0]))
print("    worst:           {}".format(elapsed_times[round((99.9/100) * REPEATS)-1]))
