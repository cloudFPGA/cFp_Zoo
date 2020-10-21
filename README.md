## cFp_Vitis

[![Doc Build Status](https://travis.ibm.com/cloudFPGA/cFp_Vitis.svg?token=8sgWzx3xuqu53CzFUy8K&branch=master)](https://travis.ibm.com/cloudFPGA/cFp_Vitis)  [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

cloudFPGA project (cFp) for Xilinx Vitis library

- [Project documentation](https://pages.github.ibm.com/cloudFPGA/Doc/pages/project_repository.html#cfp-vitis)
- [Code documentation](https://pages.github.ibm.com/cloudFPGA/Dox/group__cFp__Vitis.html)

**Idea**: The `cFp_Vitis` project bridges the accelerated libraries of open source [Xilinx Vitis](https://github.com/Xilinx/Vitis_Libraries) to [cloudFPGA](https://pages.github.ibm.com/cloudFPGA/Doc/index.html) platform.
```
   CPU(OpenCL/OpenCV)  -->  FPGA (Vitis HLS C++)  -->  FPGA (Vitis HLS C++)
    /\__________________________________________________|
```

![Oveview of cFp_Vitis](./doc/cFp_Vitis.png)


### System configurattion

#### Ubuntu

Assuming Ubuntu >16.04 the folowing packages should be installed:
```
sudo apt-get install -y build-essential pkg-config libxml2-dev python3-opencv libjpeg-dev libpng-dev libopencv-dev libopencv-contrib-dev rename rpl
```

You may also need these steps for Ubuntu 18.04 & Vitis 2019.2 :
```
sudo apt-get install libjpeg62
wget http://se.archive.ubuntu.com/ubuntu/pool/main/libp/libpng/libpng12-0_1.2.54-1ubuntu1_amd64.deb
sudo apt-get install ./libpng12-0_1.2.54-1ubuntu1_amd64.deb 
rm ./libpng12-0_1.2.54-1ubuntu1_amd64.deb
```

#### CentOS/EL7
```
sudo yum groupinstall 'Development Tools'
sudo yum install cmake opencv-devel
```

### Vivado/Vitis tool support

The versions below are supported by cFp_Vitis. As of today we follow a hybrid development approach
where a specific part of `SHELL` code is synthesized using `Vivado 2017.4`, while the rest of the 
HLS, Synthesis, P&R and bitgen are carried over with `Vivado 2019.x`.

#### For the SHELL (cFDK's code)

- [x] 2017
  - [x] 2017.4
- [ ] 2018
- [ ] 2019
- [ ] 2020

#### For the ROLE (user's code)

- [ ] 2017
- [ ] 2018
- [x] 2019
  - [x] 2019.1
  - [x] 2019.2
- [x] 2020
  - [x] 2020.1

  

##### Usefull commands

- Connect to ZYC2 network through openvpn:

  `sudo openvpn --config zyc2-vpn-user.ovpn --auth-user-pass up-user`

- Connect to a ZYC2 x86 node:

  `ssh -Y ubuntu@10.12.2.100`

- On Wireshark filter line:

  `udp.port==2718` or `tcp.port==2718`
  
- Quick bitgen:

  sometimes it accelerates the build process of `make monolithic` if:
  execute after a successfull build `make save_mono_incr` and then build the new with `make monolithic_incr` or `make monolithic_debug_incr`


  
### Vitis libraries support

The following Vitis accelerated libraries are supported by cFp_Vitis:

- [ ] BLAS
- [ ] Data Compression
- [ ] Database
- [ ] DSP
- [x] Quantitative Finance
  - [x] [MC European Engine](./ROLE/quantitative_finance/hls/mceuropeanengine)
- [ ] Security
- [ ] Solver
- [x] Vision
  - [x] [Gamma Correction](./ROLE/vision/hls/gammacorrection)
  - [x] [Harris](./ROLE/vision/hls/harris/)
