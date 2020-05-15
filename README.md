[![Build Status](https://travis.ibm.com/cloudFPGA/cFp_Vitis.svg?token=8sgWzx3xuqu53CzFUy8K&branch=master)](https://travis.ibm.com/cloudFPGA/cFp_Vitis)  [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# cFp_Vitis

cloudFPGA project (cFp) for Xilinx Vitis library

Documentation: https://pages.github.ibm.com/cloudFPGA/cFp_Vitis/

**Idea**: The `cFp_Vitis` project bridges the accelerated libraries of open source [Xilinx Vitis](https://github.com/Xilinx/Vitis_Libraries) to [cloudFPGA](https://pages.github.ibm.com/cloudFPGA/Doc/index.html) platform.
```
   CPU(OpenCL/OpenCV)  -->  FPGA (Vitis HLS C++)  -->  FPGA (Vitis HLS C++)
    /\__________________________________________________|
```

## System configurattion

Assuming Ubuntu 18.04 the folowing packages should be installed:
```
sudo apt-get install -y build-essential pkg-config libxml2-dev python3-opencv
```

## Quick divein 

![Oveview of cFp_Vitis](./doc/cFp_Vitis.png)

```bash
git clone --recursive git@github.ibm.com:cloudFPGA/cFp_Vitis.git
cd cFp_Vitis
source ./env/setenv.sh
cd ./ROLE/1/hls/harris_app
make fcsim -j 4  # to run simulation using your system's gcc (with 4 threads)
make csim   # to run simulation using Vivado's gcc
make cosim  # to run co-simulation using Vivado
make csynth # to run HLS using Vivado
make callgraph # to run fcsim and then execute the binary in Valgrind's callgraph tool
make kcachegrind # to run callgrah and then view the output in Kcachegrind tool
make memcheck # to run fcsim and then execute the binary in Valgrind's memcheck tool (to inspect memory leaks)
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
