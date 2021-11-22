cFp_Vitis Regression
====================

A regression is a suite of functional verification tests executed against the cFp_Vitis. 
Such a regression is typically called by a Jenkins server during the software development process.     

Different types of regressions can be executed by calling one of the following shell scripts:
  - `run_csim_reg.sh ` performs a HSL/C-SIMULATION of the HLS-based IP cores. 
  - `run_cosim_reg.sh` performs a RTL/CO-SIMULATION of the HLS-based IP cores.
  - `run_main_reg.sh ` sequentially calls `run_csim_reg.sh` and `run_cosim_reg.sh`.


**Warning**  
  All the above scripts must be executed from the cFp_Vitis root directory which must be defined ahead with the **$cFpVitisRootDir** variable. Any other environment variable must also be sourced beforehand.

**Example**  
The main cFp_Vitis regression script can be called as follows:
```
source /tools/Xilinx/Vivado/2020.1/settings64.sh

echo $PWD
export cFpVitisRootDir=$PWD

$cFpVitisRootDir/REG/run_main_reg.sh
```




