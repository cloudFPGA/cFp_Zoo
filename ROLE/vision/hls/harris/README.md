#### Harris Corner Detection


![Oveview of Vitis Vision Harris Corner Detector](../../../../doc/harris_overview.png)


##### Repository and environment setup

```bash
git clone --recursive git@github.ibm.com:cloudFPGA/cFp_Vitis.git
cd cFp_Vitis
source ./env/setenv.sh
```


##### Intergration of Vitis Vision Harris with cloudFPGA

In the following figure it is shown how straightforward is to intergrate a function from Vitis libraries with cloudFPGA project.

![Oveview of Vitis Vision Harris dataflow](../../../../doc/harris_dataflow.png)

Since most of Vitis libraries (in L1) are offered with a AXI stream I/F in dataflow mode, the most obvious approach to connect them to cF is to wrap this 
I/F with anohter I/F that takes care of carefully feeding (as well as sending the results back) to (from) the Harris IP from the network. 
For cFp_Vitis we are using the Themisto Shell already equipeed with a network streaming I/F for the user application. 
A small FSM takes care of the data casting between network and AXI streams.


##### Harris Simulation 

The testbench of Harris is highlighted below:

![Oveview of Vitis Vision Harris Testbench](../../../../doc/harris_tb.png)

The testbench is offered in two flavors:
- HLS TB: The testbench of the C++/RTL. This is a typical Vivado HLS testbench but it includes the testing of Harris IP when this is wrapped in a [cF Themisto Shell](https://pages.github.ibm.com/cloudFPGA/Doc/pages/cfdk.html#the-themisto-sra).
- Host TB: This includes the testing of a a host apllication (C++) that send/receives images over Ethernet (TCP/UDP) with a cF FPGA. This testbench establishes a socket-based connection with an intermediate listener which further calls the previous testbench. So practically, the 2nd tb is a wrapper of the 1st tb, but passing the I/O data over socket streams.
  For example this is the `system command` inside `Host TB` that calls the `HLS TB`:

This folder contains the mandatory files to proceed withthe 1st option, i.e. HLS TB
  
Basic files/module for the HLS TB:
  3. [test_harris_app.cpp](https://github.ibm.com/cloudFPGA/cFp_Vitis/blob/master/ROLE/vision/hls/harris_app/src/harris_app.cpp): The typical Vivado HLS testbench of Harris IP, when this is wrapped in a Themisto Shell.
  4. [Themisto Shell](https://pages.github.ibm.com/cloudFPGA/Doc/pages/cfdk.html#the-themisto-sra): The SHELL-ROLE architecture of cF.
  5. [cFp_Vitis](https://github.ibm.com/cloudFPGA/cFp_Vitis): The project that bridges Vitis libraries with cF.

  
###### Harris image size 

The maximum image size, that the Harris IP is configured, is defined at https://github.ibm.com/cloudFPGA/cFp_Vitis/blob/master/ROLE/vision/host/harris/include/config.h 
through the `FRAME_HEIGHT` and `FRAME_WIDTH` definitions. These definitions have an impact of the FPGA resources. In the following simulations if the image 
provided has other dimensions, the `cv::resize` function will be used to adjust the image (scale) to `FRAME_HEIGHT x FRAME_WIDTH`.
  
**Note:** Remember to run `make clean` every time you change those definitions.
  
###### Run simulation

**HLS TB**
  
```bash
cd ./ROLE/vision/hls/harris_app
make fcsim -j 4  # to run simulation using your system's gcc (with 4 threads)
make csim   # to run simulation using Vivado's gcc
make cosim  # to run co-simulation using Vivado
```

**Optional steps**

```bash
cd ./ROLE/vision/hls/harris_app
make callgraph # to run fcsim and then execute the binary in Valgrind's callgraph tool
make kcachegrind # to run callgrah and then view the output in Kcachegrind tool
make memcheck # to run fcsim and then execute the binary in Valgrind's memcheck tool (to inspect memory leaks)
```


##### Harris Synthesis

Since curretnly the cFDK supports only Vivado(HLS) 2017.4 we are following a 2-steps synthesis 
procedure. Firstly we synthesize the Themisto SHELL with Vivado (HLS) 2017.4 and then we synthesize 
the rest of the project (including P&R and bitgen) with Vivado (HLS) > 2019.1. 

###### The Harris IP
This is only for the HLS of Harris (e.g. to check synthesizability)
```bash
cd cFp_Vitis/ROLE/vision/hls
make harris # with Vivado HLS >= 2019.1
```
or 
```bash
cd cFp_Vitis/ROLE/vision/hls/harris
make csynth # with Vivado HLS >= 2019.1
```
or
```bash
cd cFp_Vitis/ROLE/vision/hls/harris
vivado_hls -f run_hls.tcl # with Vivado HLS >= 2019.1
```

###### The Themisto SHELL
```bash
cd cFp_Vitis/cFDK/SRA/LIB/SHELL/Themisto
make all # with Vivado HLS == 2017.4
```

###### The complete cFp_Vitis
```bash
cd cFp_Vitis
make monolithic # with Vivado HLS >= 2019.1
```

More info for the Harris IP: https://xilinx.github.io/Vitis_Libraries/vision/api-reference.html#harris-corner-detection
