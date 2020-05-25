[![Build Status](https://travis.ibm.com/cloudFPGA/cFp_Vitis.svg?token=8sgWzx3xuqu53CzFUy8K&branch=master)](https://travis.ibm.com/cloudFPGA/cFp_Vitis)  [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# cFp_Vitis

cloudFPGA project (cFp) for Xilinx Vitis library

Documentation: https://pages.github.ibm.com/cloudFPGA/cFp_Vitis/

**Idea**: The `cFp_Vitis` project bridges the accelerated libraries of open source [Xilinx Vitis](https://github.com/Xilinx/Vitis_Libraries) to [cloudFPGA](https://pages.github.ibm.com/cloudFPGA/Doc/index.html) platform.
```
   CPU(OpenCL/OpenCV)  -->  FPGA (Vitis HLS C++)  -->  FPGA (Vitis HLS C++)
    /\__________________________________________________|
```

![Oveview of cFp_Vitis](./doc/cFp_Vitis.png)


## System configurattion

Assuming Ubuntu >16.04 the folowing packages should be installed:
```
sudo apt-get install -y build-essential pkg-config libxml2-dev python3-opencv libjpeg-dev libpng-dev libopencv-dev libopencv-contrib-dev
```

You may also need these steps for Ubuntu 18.04 & Vitis 2019.2 :
```
sudo apt-get install libjpeg62
wget http://se.archive.ubuntu.com/ubuntu/pool/main/libp/libpng/libpng12-0_1.2.54-1ubuntu1_amd64.deb
sudo apt-get install ./libpng12-0_1.2.54-1ubuntu1_amd64.deb 
rm ./libpng12-0_1.2.54-1ubuntu1_amd64.deb
```


## Vitis libraries support

The following Vitis accelerated libraries are supported by cFp_Vitis:

- [ ] blas
- [ ] data_compression
- [ ] database
- [ ] dsp
- [ ] quantitative_finance
- [ ] security
- [ ] solver
- [x] vision
  - [x] harris





### Vitis Vision Harris Corner Detector dive in 

![Oveview of Vitis Vision Harris Corner Detector](./doc/harris_overview.png)


#### Repository and environment setup

```bash
git clone --recursive git@github.ibm.com:cloudFPGA/cFp_Vitis.git
cd cFp_Vitis
source ./env/setenv.sh
```


#### Intergration of Vitis Vision Harris with cloudFPGA

In the following figure it is shown how straightforward is to intergrate a function from Vitis libraries with cloudFPGA project.

![Oveview of Vitis Vision Harris dataflow](./doc/harris_dataflow.png)

Since most of Vitis libraries (in L1) are offered with a AXI stream I/F in dataflow mode, the most obvious approach to connect them to cF is to wrap this 
I/F with anohter I/F that takes care of carefully feeding (as well as sending the results back) to (from) the Harris IP from the network. 
For cFp_Vitis we are using the Themisto Shell already equipeed with a network streaming I/F for the user application. 
A small FSM takes care of the data casting between network and AXI streams.


#### Harris Simulation 

The testbench of Harris is highlighted below:

![Oveview of Vitis Vision Harris Testbench](./doc/harris_tb.png)

The testbench is offered in two flavors:
- HLS tb: The testbench of the C++/RTL. This is a typical Vivado HLS testbench but it includes the testing of Harris IP when this is wrapped in a [cF Themisto Shell](https://pages.github.ibm.com/cloudFPGA/Doc/pages/cfdk.html#the-themisto-sra).
- End-user application TB: This includes the testing of a a host apllication (C++) that send/receives images over Ethernet (TCP/UDP) with a cF FPGA. This testbench establishes a socket-based connection with an intermediate listener which further calls the previous testbench. So practically, the 2nd tb is a wrapper of the 1st tb, but passing the I/O data over socket streams.

Basic files/modules:
  1. [harris_host.cpp](https://github.ibm.com/cloudFPGA/cFp_Vitis/blob/master/ROLE/1/host/harris/src/harris_host.cpp): The end-user application. This is the application that a user can execute on a x86 host and send an image to the FPGA for processing with Harris Corner Detector algorithm. This file is part of both the `HLS tb` and the `End-user application TB`
  2. [harris_host_fw_tb.cpp](https://github.ibm.com/cloudFPGA/cFp_Vitis/blob/master/ROLE/1/host/harris/src/harris_host_fwd_tb.cpp): The intermediate listener for socket connections from an end-user application. This file is part only of the `End-user application TB`.
  3. [test_harris_app.cpp](https://github.ibm.com/cloudFPGA/cFp_Vitis/blob/master/ROLE/1/hls/harris_app/src/harris_app.cpp): The typical Vivado HLS testbench of Harris IP, when this is wrapped in a Themisto Shell.
  4. [Themisto Shell](https://pages.github.ibm.com/cloudFPGA/Doc/pages/cfdk.html#the-themisto-sra): The SHELL-ROLE architecture of cF.
  5. [cFp_Vitis](https://github.ibm.com/cloudFPGA/cFp_Vitis): The project that bridges Vitis libraries with cF.

  
##### Run simulation

###### HLS testbench
  
```bash
cd ./ROLE/1/hls/harris_app
make fcsim -j 4  # to run simulation using your system's gcc (with 4 threads)
make csim   # to run simulation using Vivado's gcc
make cosim  # to run co-simulation using Vivado
```

###### Optional steps

```bash
cd ./ROLE/1/hls/harris_app
make callgraph # to run fcsim and then execute the binary in Valgrind's callgraph tool
make kcachegrind # to run callgrah and then view the output in Kcachegrind tool
make memcheck # to run fcsim and then execute the binary in Valgrind's memcheck tool (to inspect memory leaks)
```

###### End-user application testbench
  
```bash
cd ./ROLE/1/host/harris
mkdir build && cd build
cmake ../
make -j 2
# Usage: ./harris_host_fwd_tb <Server Port> <optional simulation mode>
./harris_host_fwd_tb 1234 0
# Open another terminal and prepare env
cd cFp_Vitis
source ./env/setenv.sh
cd ./ROLE/1/host/harris/build
# Usage: ./harris_host <Server> <Server Port> <optional input image>
./harris_host localhost 1234 ../../../hls/harris_app/test/8x8.png
# You should expect the output in the file <optional input image>_fpga_out.png
eog ../../../hls/harris_app/test/8x8.png_fpga_out.png

```

#### Harris Synthesis


##### Only the Harris IP
```bash
cd cFp_Vitis/ROLE/1/hls/harris_app
make csynth # to run HLS using Vivado
```

##### The Harris IP with the Themisto SHELL
```bash
cd cFp_Vitis/ROLE/1
make all # with Vivado HLS >= 2019.1
```

##### The complete cFp_Vitis
```bash
cd cFp_Vitis
make monolithic # with Vivado HLS == 2017.4
```


#### Harris cF Demo

TODO: Flash a cF FPGA node with the generated bitstream and note down the IP of this FPGA node. e.g. assuming 192.168.1.10 and port 1234


```bash
cd ./ROLE/1/host/harris
mkdir build && cd build
cmake ../
make -j 2
cd cFp_Vitis/ROLE/1/host/harris/build
# Usage: ./harris_host <Server> <Server Port> <optional input image>
./harris_host 192.168.1.10 1234 ../../../hls/harris_app/test/8x8.png
# You should expect the output in the file <optional input image>_fpga_out.png
eog ../../../hls/harris_app/test/8x8.png_fpga_out.png

```


## Content from previous README (cFp_Build)
(keeping it here for reference, but not related to cFp_Vitis yet)

All communication goes over the *UDP/TCP port 2718*. Hence, the CPU should run:
```bash
$ ping <FPGA 1>
$ ping <FPGA 2>
# Terminal 1
nc -u <FPGA 1> 2718   # without -u for TCP
# Terminal 2
nc -lu 2718           # without -u for TCP
```

Then the packets will be send from Terminal 1 to 2.

For more details, `tcpdump -i <interface> -nn -s0 -vv -X port 2718` could be helpful.


The *Role* is the same for both FPGAs, because which destination the packets will have is determined by the `node_id`/`node_rank` and `cluster_size`
(VHDL ports`piFMC_ROLE_rank` and `piFMC_ROLE_size`).


The **Role forwards the packet always to `(node_rank + 1) % cluster_size`** (for UDP and TCP packets), so this example works also for more or less then two FPGAs, actually.



For distributing the routing tables, **`POST /cluster`** must be used.
The following depicts an example API call, assuming that the cFp_Triangle bitfile was uploaded as image`d8471f75-880b-48ff-ac1a-baa89cc3fbc9`:
![POST /cluster example](./doc/post_cluster.png)

## Firewall issues

Some firewalls may block network packets if there is not a connection to the remote machine/port.
Hence, to get the Triangle example to work, the following commands may be necessary to be executed (as root):
```
$ firewall-cmd --zone=public --add-port=2718-2750/udp --permanent
$ firewall-cmd --reload
```

Also, ensure that the network secuirty group settings are updated (e.g. in case of the ZYC2 OpenStack).
