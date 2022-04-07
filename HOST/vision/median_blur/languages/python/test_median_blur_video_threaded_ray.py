import ray
from ray.util.queue import Queue

import sys
import os
import numpy as np
import cv2 as cv
import pims

ray.init(dashboard_port=50051, num_cpus=8)

image_file=os.environ['cFpRootDir'] + "ROLE/vision/hls/harris/test/512x512.png"
frame = cv.imread(image_file, cv.IMREAD_COLOR)

# You can pass this object around to different tasks/actors
fpgas_queue = Queue(maxsize=100)

@ray.remote
def consumer(i, fpgas_queue, frame):
    next_item = fpgas_queue.get(block=True, timeout=100)
    print(f"will work on {next_item} and then put in back in the fpgas_queue")
    #for l in range(10000000):
    #    k = i + l
    frame_ret = cv.medianBlur(frame, 9)
    fpgas_queue.put(next_item)
    print(f"finished working on {next_item} Now it is back in the fpgas_queue")    
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

print("length of ")


consumers = [consumer.remote(i, fpgas_queue, frames[i]) for i in range(len(frames))]

[fpgas_queue.put(j) for j in ([["10.12.200.73" , "2718"],
                        ["10.12.200.24" , "2719"],
                        ["10.12.200.11" , "2720"],
                        ["10.12.200.19" , "2721"],
                        ["10.12.200.29" , "2722"]])]

results = ray.get(consumers)
print('Tasks executed')

video_name = str(fn)+"_out.avi"
video_out = cv.VideoWriter(video_name, cv.VideoWriter_fourcc('M','J','P','G'), 30, (results[0].shape[1],results[0].shape[0]))
for t in range(len(results)):
    video_out.write(results[t])
video_out.release()
print("Saved video: " + video_name)
