cloudFPGA Regression
====================

A regression is a suite of functional verification tests executed against the cFp_Triangle project. Such a regression is typically called by a Jenkins server during the software development process.

A regression is executed by calling a shell script such as:
  - `run_main.sh` which performs a '_build monolithic_' of the FPGA followed by a RTL/CO-SIMULATION of the HLS-based IP cores.

**Warning**
The above scripts must be executed from the cFp_Triangle root directory which must be defined ahead with the _$cFpRootDir_ variable. Any other environment variable must also be sourced beforehand.

**Example** 
The main cFp_Triangle script can be called as follows:

```
source /tools/Xilinx/Vivado/2017.4/settings64.sh

echo $PWD
export cFpRootDir=$PWD

$cFpRootDir/REG/run_main.sh
```

**About the _run_main_ script**  
The `run_main.sh` script is the main entry point for running a regression. It sets all necessary cF-environment variables, like a typical `env/setenv.sh` script would do. Next, if other scripts are to be run, they are expected to be called by `run_main.sh`.


