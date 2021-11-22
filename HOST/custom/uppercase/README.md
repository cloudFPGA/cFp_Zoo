# Demo

TODO: Flash a cF FPGA node with the generated bitstream and note down the IP of this FPGA node. e.g. assuming `10.12.200.153` and port `2718`


```bash
cd ./HOST
mkdir build && cd build
cmake ../
make -j 2
# Usage: ./uppercase_host <Server> <Server Port> <input string>
./uppercase_host 10.12.200.153 2718 "HelloWorld"
# You should expect the output in the stdout

```
## Acknowledgement and Copyright
This project is built upon various open-sourced libraries, like [Practical C++ Sockets](http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/) and [OpenCV 3](http://opencv.org/) ; please refer to their original license accordingly (GPL/BSD). Code of this project is puslished under Apache v2 License.
