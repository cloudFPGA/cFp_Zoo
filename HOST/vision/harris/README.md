# Demo

TODO: Flash a cF FPGA node with the generated bitstream and note down the IP of this FPGA node. e.g. assuming `10.12.200.153` and port `2718`


```bash
cd ./ROLE/vision/host/harris
mkdir build && cd build
cmake ../
make -j 2
cd cFp_Vitis/ROLE/vision/host/harris/build
# Usage: ./harris_host <Server> <Server Port> <optional input image>
./harris_host 10.12.200.153 2718 ../../../../ROLE/vision/hls/harris/test/8x8.png
# You should expect the output in the file <optional input image>_fpga_out_frame_#.png
eog ../../../../ROLE/vision/hls/harris/test/8x8.png_fpga_points_out_frame_1.png

```
## Acknowledgement and Copyright
This project is built upon various open-sourced libraries, like [Practical C++ Sockets](http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/) and [OpenCV 3](http://opencv.org/) ; please refer to their original license accordingly (GPL/BSD). Code of this project is puslished under Apache v2 License.
