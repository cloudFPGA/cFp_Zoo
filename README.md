# cFp_Zoo Overview

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

cFp_Zoo is a cloudFPGA project (cFp) featuring domain-specific accelerators for the hybrid multi-cloud era.

- [Project documentation](https://cloudfpga.github.io/Doc/pages/PROJECTS/cFp_Zoo_overview.html)
- [Code documentation](https://cloudfpga.github.io/Dox/group__cFp__Zoo.html)

**Idea**: The `cFp_Zoo` project develops numerous domain-specific accelerators, incuding those from the open source library [Vitis Libraries](https://github.com/Xilinx/Vitis_Libraries) to the [cloudFPGA](https://cloudfpga.github.io/Doc/index.html) platform.


![Oveview of cFp_Zoo](./doc/cFp_Zoo.png)


## System configurattion

### Ubuntu

Assuming Ubuntu >16.04 the folowing packages should be installed:
```
sudo apt-get install -y build-essential pkg-config libxml2-dev python3-opencv libjpeg-dev libpng-dev libopencv-dev libopencv-contrib-dev rename rpl dialog cmake swig python3-dev python3.8-venv gcc-multilib(=for vitis)
```

You may also need these steps for Ubuntu 18.04 & Vitis 2019.2 :
```
sudo apt-get install libjpeg62
wget http://se.archive.ubuntu.com/ubuntu/pool/main/libp/libpng/libpng12-0_1.2.54-1ubuntu1_amd64.deb
sudo apt-get install ./libpng12-0_1.2.54-1ubuntu1_amd64.deb 
rm ./libpng12-0_1.2.54-1ubuntu1_amd64.deb
```

### CentOS/EL7
```
sudo yum groupinstall 'Development Tools'
sudo yum install cmake opencv-devel dialog python-numpy libxml2-devel python3 wireshark wireshark-gnome xauth rpl
```

## Vivado/Vitis tool support

The versions below are supported by cFp_Zoo.

#### For the SHELL (cFDK's code)

- [x] 2017
  - [x] 2017.4
- [x] 2018
- [x] 2019
  - [x] 2019.1
  - [x] 2019.2 
- [X] 2020
  - [x] 2020.1
  - [ ] 2020.2
- [ ] 2021

### For the ROLE (user's code)

- [ ] 2017
- [ ] 2018
- [x] 2019
  - [x] 2019.1
  - [x] 2019.2
- [x] 2020
  - [x] 2020.1
  - [ ] 2020.2
- [ ] 2021



## Vitis libraries support

The following Vitis accelerated libraries are supported by cFp_Zoo:

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
  - [x] [MedianBlur](./ROLE/vision/hls/median_blur/)

  
  

## Quick cFp_Zoo configuration
![Step 1 - Terminal](./doc/config1.png)
![Step 2 - Info](./doc/config2.png)
![Step 3 - Load configuration](./doc/config3.png)
![Step 4 - Select TCP/UDP](./doc/config4.png)
![Step 5 - Select Domain](./doc/config5.png)
![Step 6 - Select Kernel](./doc/config6.png)
![Step 7 - Select MTU](./doc/config7.png)
![Step 8 - Select Port](./doc/config8.png)
![Step 9 - Select DDR](./doc/config9.png)
![Step 10 - Confirm](./doc/config10.png)
![Step 11 - Finish](./doc/config11.png)


## cFp_Zoo Essentials

#### Firewall issues

Some firewalls may block network packets if there is not a connection to the remote machine/port.
Hence, to get the Triangle example to work, the following commands may be necessary to be executed 
(as root):

```
$ firewall-cmd --zone=public --add-port=2718-2750/udp --permanent
$ firewall-cmd --zone=public --add-port=2718-2750/tcp --permanent
$ firewall-cmd --reload
```

Also, ensure that the network secuirty group settings are updated (e.g. in case of the ZYC2 OpenStack).



### Usefull commands

- Connect to ZYC2 network through openvpn:

  `sudo openvpn --config zyc2-vpn-user.ovpn --auth-user-pass up-user`

- Connect to a ZYC2 x86 node:

  `ssh -Y ubuntu@10.12.2.100`

- On Wireshark filter line:

  `udp.port==2718` or `tcp.port==2718`

  `ip.addr == 10.12.200.0/24`
  
- Set maximum net buffer:

  - `sudo sysctl -w net.core.rmem_max=2147483647`
  - On the host code (cpp)
    ```
    //increase buffer size
    int recvBufSize = 0x1000000;
    int err = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvBufSize, sizeof(recvBufSize));
    if(err != 0)
    {
      std::cerr <<" error socket buffer: " << err << std::endl;
      exit(EXIT_FAILURE);
    }
    int real_buffer_size = 0;
    socklen_t len2 = sizeof(real_buffer_size);
    err = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &real_buffer_size, &len2);
    printf("got %d as buffer size (requested %d)\n",real_buffer_size/2, recvBufSize);
    if(real_buffer_size/2 != recvBufSize)
    {
      std::cerr << "set SO_RCVBUF failed! got only: " << real_buffer_size/2 << "; trying to continue..." << std::endl;
    }
  ```

- Quick bitgen:

  sometimes it accelerates the build process of `make monolithic` if:
  execute after a successfull build `make save_mono_incr` and then build the new with `make monolithic_incr` or `make monolithic_debug_incr`
  
- Update subrepositories (e.g. for Vitis_Libraries)

  On the pc that you want to change the subrepo to a new version
  ```
  cd Vitis_Libraries
  git checkout origin master` (or any other version)
  git commit -am "Updated Vitis_Libraries to master"
  ```
  On the pc you want to sync with the new subrepo
  ```
  git submodule update --init -- Vitis_Libraries/
  ```
- Add user to wireshark group in order to capture packets without advanced privileges. (needs logout)
  ```
  sudo usermod -aG wireshark $USER
  ```
