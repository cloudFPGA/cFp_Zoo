#### Harris Host Testbench

The testbench of Harris is highlighted below:

![Oveview of Vitis Vision Harris Testbench](../../../doc/harris_tb.png)

The testbench is offered in two flavors:
- HLS TB: The testbench of the C++/RTL. This is a typical Vivado HLS testbench but it includes the testing of Harris IP when this is wrapped in a [cF Themisto Shell](https://github.com/cloudFPGA/cFDK/blob/main/DOC/Themisto.md).
- Host TB: This includes the testing of a a host apllication (C++) that send/receives images over Ethernet (TCP/UDP) with a cF FPGA. This testbench establishes a socket-based connection with an intermediate listener which further calls the previous testbench. So practically, the 2nd tb is a wrapper of the 1st tb, but passing the I/O data over socket streams.
  For example this is the `system command` inside `Host TB` that calls the `HLS TB`:
  
  ```c
  // Calling the actual TB over its typical makefile procedure, but passing the save file
  string str_command = "cd ../../../../ROLE/vision/hls/harris/ && " + clean_cmd + "\
  INPUT_IMAGE=./test/input_from_udp_to_fpga.png " + exec_cmd + " && \
  cd ../../../../HOST/vision/harris/build/ "; 
  const char *command = str_command.c_str(); 
  cout << "Calling TB with command:" << command << endl; 
  system(command); 
  ```

This folder contains the mandatory files to proceed with the 2nd option, i.e. Host TB

Basic files/modules for the Host TB:
  1. [harris_host.cpp](https://github.com/cloudFPGA/cFp_Zoo/blob/master/HOST/vision/harris/languages/cplusplus/src/harris_host.cpp): The end-user application. This is the application that a user can execute on a x86 host and send an image to the FPGA for processing with Harris Corner Detector algorithm. This file is part of both the `HLS TB` and the `Host TB`
  2. [harris_host_fw_tb.cpp](https://github.com/cloudFPGA/cFp_Zoo/blob/master/HOST/vision/harris/languages/cplusplus/src/harris_host_fwd_tb.cpp): The intermediate listener for socket connections from an end-user application. This file is part only of the `Host TB`.
  3. [test_harris.cpp](https://github.com/cloudFPGA/cFp_Zoo/blob/master/ROLE/vision/hls/harris/test/test_harris.cpp): The typical Vivado HLS testbench of Harris IP, when this is wrapped in a Themisto Shell.
  4. [Themisto Shell](https://github.com/cloudFPGA/cFDK/blob/main/DOC/Themisto.md): The Themisto SHELL-ROLE architecture of cF.
  5. [cFp_Zoo](https://github.com/cloudFPGA/cFp_Zoo): The project that bridges Vitis libraries with cF.

  
```bash
# Compile sources
cd ./HOST/vision/harris
mkdir build && cd build
cmake ../
make -j 2

# Start the intermediate listener
# Usage: ./harris_host_fwd_tb <Server Port> <optional simulation mode>
./harris_host_fwd_tb 1234 0

# Start the actual user application on host
# Open another terminal and prepare env
cd cFp_Zoo
source ./env/setenv.sh
cd ./HOST/vision/harris/build
# Usage: ./harris_host <Server> <Server Port> <optional input image>
./harris_host localhost 1234 ../../../../ROLE/vision/hls/harris/test/8x8.png

# What happens is that the user application (harris_host) is sending an input image file to 
# intermediate listener (harris_host_fwd_tb) through socket. The latter receives the payload and 
# reconstructs the image. Then it is calling the HLS TB by firstly compiling the HLS TB files. The 
# opposite data flow is realized for taking the results back and reconstruct the FPGA output image.
# You should expect the output in the file <optional input image>_fpga_out_frame_#.png
eog ../../../../ROLE/vision/hls/harris/test/8x8.png_fpga_points_out_frame_1.png

```


#### Harris cF End-to-End Demo

TODO: Flash a cF FPGA node with the generated bitstream and note down the IP of this FPGA node. e.g. assuming `10.12.200.153` and port `2718`


```bash
cd ./ROLE/vision/host/harris
mkdir build && cd build
cmake ../
make -j 2
cd cFp_Zoo/ROLE/vision/host/harris/build
# Usage: ./harris_host <Server> <Server Port> <optional input image>
./harris_host 10.12.200.153 2718 ../../../../ROLE/vision/hls/harris/test/8x8.png
# You should expect the output in the file <optional input image>_fpga_out_frame_#.png
eog ../../../../ROLE/vision/hls/harris/test/8x8.png_fpga_points_out_frame_1.png
```

**NOTE:** The cFp_Zoo ROLE (FPGA part) is equipped with both the UDP and TCP offload engines. At 
runtime, on host, to select one over the other, you simply need to change in [config.h](https://github.com/cloudFPGA/cFp_Zoo/blob/master/HOST/vision/harris/languages/cplusplus/include/config.h) 
file the define `#define NET_TYPE udp` (choose either udp or tcp).


#### Harris usefull commands

- Editing videos for input to the Harris example:
  
  `ffmpeg -i The_Mast_Walk_by_Alex_Thomson.mp4 -ss 00:00:39 -t 00:00:17 -async 1 -strict -2 cut.mp4 -c copy`
  `frame= 1025 fps= 42 q=-1.0 Lsize=   10487kB time=00:00:41.00 bitrate=2095.0kbits/s   `
  `ffmpeg -i cut.mp4 -filter:v "crop=720:720:200:20" -strict -2 cut_720x720.mp4`

  
#### Working with ZYC2

All communication goes over the *UDP/TCP port 2718*. Hence, the CPU should run:
```bash
$ ./harris_host <Server> <Server Port> <optional input image>
```

The packets will be send from Host (CPU) Terminal 1 to FPGA and they will be received back in the 
same terminal by a single host application using the `sendTo()` and `receiveFrom()` socket methods.

For more details, `tcpdump -i <interface> -nn -s0 -vv -X port 2718` could be helpful.

The *Role* can be replicated to many FPGA nodes in order to create a pipline of processing.
Which destination the packets will have is determined by the `node_id`/`node_rank` and `cluster_size`
(VHDL ports`piFMC_ROLE_rank` and `piFMC_ROLE_size`).

The **Role can be configured to forward the packet always to `(node_rank + 1) % cluster_size`** 
(for UDP and TCP packets), so this example works also for more or less then two FPGAs, actually.
curretnly, the default example supports one CPU node and one FPGA node.


##### Acknowledgement and Copyright
This software part of this project is built upon various open-source libraries, like [Practical C++ Sockets](http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/) and [OpenCV 3](http://opencv.org/) ; please refer to their original license accordingly (GPL/BSD). Code of this project is puslished under Apache v2 License.
