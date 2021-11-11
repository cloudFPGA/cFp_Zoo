# Demo

TODO: Flash a cF FPGA node with the generated bitstream and note down the IP of this FPGA node. e.g. assuming `10.12.200.153` and port `2718`

1. Build the application
2. Execute either the interactive or benchmarking mode

```bash
cd ./HOST/custom/memtest/languages/cplusplus
mkdir build && cd build
cmake ../
make
# Usage: ./memtest_host <Server> <Server Port> <number of address to test> <testing times> <burst size> <optional list/interactive mode (type list or nothing)>
./memtest_host 10.12.200.153 2718 4096 2 512
#interactive mode
# You should expect the output in the stdout and a log in a csv file for both average results and single tests
./memtest_host 10.12.200.153 2718 1 1 1 list
# benchmarking mode, running for a fixed number of times the benchmark from the biggest burst size to the shortest
# on incremental number of addresses
# to change the benchmark value
#define MAX_MEM_SIZE_BENCHMARKING_POWER_OF_TWO 33
#define MIN_MEM_SIZE_BENCHMARKING_POWER_OF_TWO 6
#define MAX_BURST_SIZE_BENCHMARKING 512
#define MIN_BURST_SIZE_BENCHMARKING 1
#define REPETITIONS_BENCHMARKING 2
```
# Emulation
To emulate your application layer, employ a machine where you can have Vivado HLS library installed and all the deployment stuffs
1. build everything as before (Before running the `memtest_host` executable)
2. Execute the emulator (an infinite loop) as:
    ``` ./memtest_host_fwd_tb 2718 2 ```
    `Usage: ./memtest_host_fwd_tb <Server Port> <optional simulation mode>`
3. Run as before the host in the interactive or benchmarking mode

# Data Visualization
1. Execute some tests will produce two `cfp_vitis_memtest_*.csv` an `avg` and a `multi` for average or single detailed iterations results.
2. copy the data to `segretini-matplottini/data`
3. Currently two dataset are required. One to compare the simple and the complex memory test version (`memtest_smpl_vs_cmplx.csv` data) and the scaling dataset of the complex version (`memtest_plot_complex.csv`). For the former dataset, add manually `TestVersion` column to distinguish the simple and the complex version
4. Navigate to `segretini-matplottini/src/examples`
5. Execute the plotter `python3 membw.py`. 
6. Find the updated data in the `plots` folder :)

## Acknowledgement and Copyright
This project is built upon various open-sourced libraries, like [Practical C++ Sockets](http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/) and [OpenCV 3](http://opencv.org/) ; please refer to their original license accordingly (GPL/BSD). Code of this project is puslished under Apache v2 License.
