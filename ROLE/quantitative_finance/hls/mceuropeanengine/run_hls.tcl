# *****************************************************************************
# *                            cloudFPGA
# *            All rights reserved -- Property of IBM
# *----------------------------------------------------------------------------
# * Created : Sep 2018
# * Authors : Francois Abel, Dionysios Diamantopoulos
# *
# * Description : A Tcl script for the HLS batch syhthesis of the UDP applica-
# *   tion embedded into the Flash of the cloudFPGA ROLE.
# *
# * Synopsis : vivado_hls -f <this_file>
# *
# *
# * Reference documents:
# *  - UG902 / Ch.4 / High-Level Synthesis Reference Guide.
# *
# *-----------------------------------------------------------------------------
# * Modification History:
# *  Fab: Jan-18-2018 Adds header and environment variables.
# *  Fab: Feb-15-2018 Changed the export procedure.
# *  Did: Apr-22-2020 Added xf:cv:mceuropeanengine example from Xilinx Vitis
# ******************************************************************************

# User defined settings
#-------------------------------------------------
set projectName    "mceuropeanengine"
set solutionName   "solution1"
set xilPartName    "xcku060-ffva1156-2-i"

set ipName         ${projectName}
set ipDisplayName  "MCEuropeanEngine Application Example."
set ipDescription  "Demonstrates the functionalities of a cloudFPGA cluster."
set ipVendor       "IBM"
set ipLibrary      "hls"
set ipVersion      "1.0"
set ipPkgFormat    "ip_catalog"
set ipRtl          "vhdl"

# Set Project Environment Variables
#-------------------------------------------------
set currDir      [pwd]
set srcDir       ${currDir}/src
set incDir       ${currDir}/include
set testDir      ${currDir}/test
set implDir      ${currDir}/${projectName}_prj/${solutionName}/impl/ip
set repoDir      ${currDir}/../../ip

# Get targets out of env
#-------------------------------------------------
set hlsSim $env(hlsSim)
set hlsCoSim $env(hlsCoSim)
set hlsSyn $env(hlsSyn)

# Open and Setup Project
#-------------------------------------------------
open_project  ${projectName}_prj
set_top       ${projectName}
#set_top cornerMCEuropeanEngine_accel
#set_top mceuropeanengine_accel

set vitis_flags  "-D__SDSVHLS__ -std=c++0x"

if { $hlsSim} {
  set hlslib_flags "-std=c++11 "
}
if { $hlsSyn || $hlsCoSim}  {
  set hlslib_flags "-std=c++11 -DHLSLIB_SYNTHESIS"
}
# the -I flag without trailing '/'!!
add_files     ${srcDir}/kernel_mc.cpp -cflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls ${vitis_flags} -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L1/include -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L2/include ${hlslib_flags}" -csimflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L1/include -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L2/include ${vitis_flags} ${hlslib_flags}"
add_files     ${srcDir}/${projectName}.cpp -cflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L1/include -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L2/include  ${vitis_flags} ${hlslib_flags}" -csimflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L1/include -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L2/include ${vitis_flags} ${hlslib_flags}"
add_files -tb ${testDir}/test_${projectName}.cpp -cflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L1/include -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L2/include ${vitis_flags}" -csimflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L1/include -I$env(cFpRootDir)/Vitis_Libraries/quantitative_finance/L2/include ${vitis_flags}"

# Create a solution
#-------------------------------------------------
open_solution ${solutionName}

set_part      ${xilPartName}
create_clock -period 6.4 -name default

# Run C Simulation and Synthesis
#-------------------------------------------------
if { $hlsSim } {
  csim_design -O -compiler gcc
} else {

  if { $hlsSyn } {
    csynth_design
  }
  
  if { $hlsCoSim } {
    cosim_design -compiler gcc -trace_level all
  } else {

  # Export RTL
  #-------------------------------------------------
    export_design -rtl vhdl -format ${ipPkgFormat} -library ${ipLibrary} -display_name ${ipDisplayName} -description ${ipDescription} -vendor ${ipVendor} -version ${ipVersion}
  }
}

# Exit Vivado HLS
#--------------------------------------------------
exit
