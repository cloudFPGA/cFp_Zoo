### cFp WarpTransform Filter

This is the implementation of the common Median Blur Filter (WarpTransform), on cloudFPGA platform. 
The WarpTransform IP is privided by the open source Xilinx ® Vitis™ Vision library, which is a fundamental library aimed at providing a comprehensive FPGA acceleration library for computer vision algorithms. 

![Oveview of Vitis Vision WarpTransform Corner Detector](../../../../doc/warp_transform_overview.png)


#### Repository and environment setup

```bash
git clone --recursive git@github.ibm.com:cloudFPGA/cFp_Zoo.git
cd cFp_Zoo
source ./env/setenv.sh
```


#### Intergration of Vitis Vision WarpTransform with cloudFPGA

In the following figure it is shown how straightforward is to intergrate a function from Vitis libraries with cloudFPGA project.

![Oveview of Vitis Vision WarpTransform dataflow](../../../../doc/warp_transform_dataflow.png)

Since most of Vitis libraries (in L1) are offered with a AXI stream I/F in dataflow mode, the most obvious approach to connect them to cF is to wrap this 
I/F with anohter I/F that takes care of carefully feeding (as well as sending the results back) to (from) the WarpTransform IP from the network. 
For cFp_Zoo we are using the Themisto Shell already equipeed with a network streaming I/F for the user application. 
A small FSM takes care of the data casting between network and AXI streams.


#### WarpTransform Simulation 

The testbench of WarpTransform is highlighted below:

![Oveview of Vitis Vision WarpTransform Testbench](../../../../doc/warp_transform_tb.png)

The testbench is offered in two flavors:
- HLS TB: The testbench of the C++/RTL. This is a typical Vivado HLS testbench but it includes the testing of WarpTransform IP when this is wrapped in a [cF Themisto Shell](https://pages.github.ibm.com/cloudFPGA/Doc/pages/cfdk.html#the-themisto-sra).
- Host TB: This includes the testing of a a host apllication (C++) that send/receives images over Ethernet (TCP/UDP) with a cF FPGA. This testbench establishes a socket-based connection with an intermediate listener which further calls the previous testbench. So practically, the 2nd tb is a wrapper of the 1st tb, but passing the I/O data over socket streams.
  For example this is the `system command` inside `Host TB` that calls the `HLS TB`:

This folder contains the mandatory files to proceed withthe 1st option, i.e. HLS TB
  
Basic files/module for the HLS TB:
  3. [test_warp_transform.cpp](https://github.com/cloudFPGA/cFp_Zoo/blob/master/ROLE/vision/hls/warp_transform/test/test_warp_transform.cpp): The typical Vivado HLS testbench of Harris IP, when this is wrapped in a Themisto Shell.
  4. [Themisto Shell](https://github.com/cloudFPGA/cFDK/blob/main/DOC/Themisto.md): The SHELL-ROLE architecture of cF.
  5. [cFp_Zoo](https://github.com/cloudFPGA/cFp_Zoo): The project that bridges Vitis libraries with cF.

  
  
##### WarpTransform image size 

The maximum image size, that the WarpTransform IP is configured, is defined at https://github.com/cloudFPGA/cFp_Zoo/blob/master/HOST/vision/warp_transform/languages/cplusplus/include/config.h
through the `FRAME_HEIGHT` and `FRAME_WIDTH` definitions. These definitions have an impact of the FPGA resources. In the following simulations if the image 
provided has other dimensions, the `cv::resize` function will be used to adjust the image (scale) to `FRAME_HEIGHT x FRAME_WIDTH`.
  
**Note:** Remember to run `make clean` every time you change those definitions.
  
##### Run simulation

**HLS TB**
  
```bash
cd ./ROLE/vision/hls/warp_transform_app
make fcsim -j 4  # to run simulation using your system's gcc (with 4 threads)
make csim   # to run simulation using Vivado's gcc
make cosim  # to run co-simulation using Vivado
```

**Optional steps**

```bash
cd ./ROLE/vision/hls/warp_transform_app
make callgraph # to run fcsim and then execute the binary in Valgrind's callgraph tool
make kcachegrind # to run callgrah and then view the output in Kcachegrind tool
make memcheck # to run fcsim and then execute the binary in Valgrind's memcheck tool (to inspect memory leaks)
```


#### WarpTransform Synthesis

Since curretnly the cFDK supports only Vivado(HLS) 2017.4 we are following a 2-steps synthesis 
procedure. Firstly we synthesize the Themisto SHELL with Vivado (HLS) 2017.4 and then we synthesize 
the rest of the project (including P&R and bitgen) with Vivado (HLS) > 2019.1. 

##### The WarpTransform IP
This is only for the HLS of WarpTransform (e.g. to check synthesizability)
```bash
cd cFp_Zoo/ROLE/vision/hls
make warp_transform # with Vivado HLS >= 2019.1
```
or 
```bash
cd cFp_Zoo/ROLE/vision/hls/warp_transform
make csynth # with Vivado HLS >= 2019.1
```
or
```bash
cd cFp_Zoo/ROLE/vision/hls/warp_transform
vivado_hls -f run_hls.tcl # with Vivado HLS >= 2019.1
```

##### The Themisto SHELL
```bash
cd cFp_Zoo/cFDK/SRA/LIB/SHELL/Themisto
make all # with Vivado HLS == 2019.1
```

##### The complete cFp_Zoo
```bash
cd cFp_Zoo
make monolithic # with Vivado HLS >= 2019.1
```

More info for the WarpTransform IP: https://xilinx.github.io/Vitis_Libraries/vision/2020.1/api-reference.html#median-blur-filter


##### Troubleshooting

* ```
  Vivado libstdc++.so.6: version CXXABI_1.3.11 not found (required by /lib64/libtbb.so.2)`
  ```
  Fix: `cp /usr/lib64/libstdc++.so.6 /tools/Xilinx/Vivado/2020.1/lib/lnx64.o/Default/libstdc++.so.6`

*
  ```
  /lib64/libtbb.so.2: undefined reference to `__cxa_init_primary_exception@CXXABI_1.3.11'
  /lib64/libtbb.so.2: undefined reference to `std::__exception_ptr::exception_ptr::exception_ptr(void*)@CXXABI_1.3.11'
  ```
  Red Hat Fix: `csim_design -ldflags "-L/usr/lib/gcc/x86_64-redhat-linux/8/ ${OPENCV_LIB_FLAGS} ${OPENCV_LIB_REF}" -clean -argv "${SimFile}"`
  Ubuntu Fix: `csim_design -ldflags "-L/usr/lib/x86_64-linux-gnu/ ${OPENCV_LIB_FLAGS} ${OPENCV_LIB_REF}" -clean -argv "${SimFile}"`
*
  ```
/usr/include/features.h:367:12: fatal error: 'sys/cdefs.h' file not found # include <sys/cdefs.h>
```
Fix: `sudo apt-get install gcc-multilib g++-multilib`

* Inability to compile vitis libraries for some libs.
Add this fix to the Xilinx makefile
Fix: ```LDFLAGS += `pkg-config --libs opencv` `xml2-config --cflags --libs```

* For recompiling opencv one possibility
```cmake -D CMAKE_BUILD_TYPE=RELEASE \
-D CMAKE_INSTALL_PREFIX=$cwd/installation/OpenCV-"$cvVersion" \
-D INSTALL_C_EXAMPLES=ON \
-D INSTALL_PYTHON_EXAMPLES=ON \
-D WITH_TBB=ON \
-D WITH_V4L=ON \
-D OPENCV_SKIP_PYTHON_LOADER=ON \
-D OPENCV_GENERATE_PKGCONFIG=ON \
-D WITH_QT=ON \
-D WITH_OPENGL=ON \
-D OPENCV_PYTHON3_INSTALL_PATH=$cwd/OpenCV-$cvVersion-py3/lib/python3.5/site-packages \
-D PYTHON_DEFAULT_EXECUTABLE=/usr/bin/python3 \
-D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
-D ENABLE_CXX11=ON \
-D BUILD_EXAMPLES=ON \
-D OPENCV_GENERATE_PKGCONFIG=ON ../opencv ..
```

* Gmph issue.
  ```
  /tools/Xilinx/Vivado/2020.1/include/mpfr.h:181:9: error: ‘__gmp_const’ does not name a type
  typedef __gmp_const __mpfr_struct *mpfr_srcptr;
  ```
  Fix: add the following lines to mpfr.h header of Vivado
`
#ifndef __GMP_H__
  #include <gmp.h>
#else
  #include "/home/hoangt/TOOLS/xilinx/Vivado/2020.1/include/gmp.h"
  #ifndef __gmp_const
    #define __gmp_const const
  #endif
#endif
'
