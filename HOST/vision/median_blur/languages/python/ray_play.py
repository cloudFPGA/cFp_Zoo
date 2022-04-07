import ray
from ray.util.queue import Queue

import sys
import os
import numpy as np
import cv2 as cv

ray.init(dashboard_port=50051, num_cpus=8)

image_file=os.environ['cFpRootDir'] + "ROLE/vision/hls/harris/test/512x512.png"
frame = cv.imread(image_file, cv.IMREAD_COLOR)

# You can pass this object around to different tasks/actors
queue = Queue(maxsize=100)

@ray.remote
def consumer(i, queue, frame):
    next_item = queue.get(block=True, timeout=100)
    print(f"will work on {next_item} and then put in back in the queue")
    for l in range(10000000):
        k = i + l
    frame_ret = cv.medianBlur(frame, 9)
    queue.put(next_item)
    return frame_ret


consumers = [consumer.remote(i, queue, frame) for i in range(5)]

[queue.put(j) for j in ([["10.12.200.73" , "2718"],
                        ["10.12.200.24" , "2719"],
                        ["10.12.200.11" , "2720"],
                        ["10.12.200.19" , "2721"],
                        ["10.12.200.29" , "2722"]])]


results = ray.get(consumers)
print('Tasks executed')

for t in range(len(results)):
    file_out = "/tmp/img_"+str(t)+".png"
    cv.imwrite(file_out, results[t])
