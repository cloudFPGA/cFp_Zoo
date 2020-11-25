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
# *  Did: Apr-22-2020 Added xf:cv:harris example from Xilinx Vitis
# ******************************************************************************

# ------------------------------------------------------------------------------
# User defined settings
# ------------------------------------------------------------------------------
set projectName    "harris"
set solutionName   "solution1"
set xilPartName    "xcku060-ffva1156-2-i"

set ipName         ${projectName}
set ipDisplayName  "Harris Application Example."
set ipDescription  "Demonstrates the functionalities of a cloudFPGA cluster."
set ipVendor       "IBM"
set ipLibrary      "hls"
set ipVersion      "1.0"
set ipPkgFormat    "ip_catalog"
set ipRtl          "vhdl"

# ------------------------------------------------------------------------------
# Set Project Environment Variables
# ------------------------------------------------------------------------------
set currDir      [pwd]
set srcDir       ${currDir}/src
set incDir       ${currDir}/include
set testDir      ${currDir}/test
set implDir      ${currDir}/${projectName}_prj/${solutionName}/impl/ip
set repoDir      ${currDir}/../../ip

# ------------------------------------------------------------------------------
# Get targets out of env
# ------------------------------------------------------------------------------
set hlsSim $env(hlsSim)
set hlsCoSim $env(hlsCoSim)
set hlsSyn $env(hlsSyn)

if { [info exists env(SimFile)] } {
  set SimFile $env(SimFile)
}

# ------------------------------------------------------------------------------
# Vitis Vision and OpenCV Libary Path Information
# ------------------------------------------------------------------------------
set cmd "pkg-config --cflags-only-I opencv"
#For Tcl versions less than 8.5, this will work
#puts [ eval exec $cmd ] 
set OPENCV_INCLUDE [ exec {*}$cmd ] 
set cmd "pkg-config --libs-only-L opencv"
set OPENCV_LIB [ exec {*}$cmd ] 

# ------------------------------------------------------------------------------
# OpenCV C Simulation / CoSimulation Library References
#------------------------------------------------------------------------------
set VISION_INC_FLAGS "-I$env(cFpRootDir)/Vitis_Libraries/vision/L1/include -std=c++0x"
set OPENCV_INC_FLAGS "-I$OPENCV_INCLUDE"
set OPENCV_LIB_FLAGS "-L $OPENCV_LIB"

# Linux OpenCV Linking Style:
set OPENCV_LIB_REF   "-lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lopencv_highgui -lopencv_flann -lopencv_features2d"

# ------------------------------------------------------------------------------
# Open and Setup Project
# ------------------------------------------------------------------------------
open_project  ${projectName}_prj
set_top       ${projectName}
#set_top cornerHarris_accel
#set_top harris_accel

set vitis_flags  "-D__SDSVHLS__ -std=c++0x"

if { $hlsSim} {
  set hlslib_flags "-std=c++11 "
}
if { $hlsSyn || $hlsCoSim}  {
  set hlslib_flags "-std=c++11 -DHLSLIB_SYNTHESIS"
}
# the -I flag without trailing '/'!!
add_files     ${srcDir}/${projectName}.cpp -cflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls ${VISION_INC_FLAGS} ${vitis_flags} ${hlslib_flags}" -csimflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls ${VISION_INC_FLAGS} ${vitis_flags} ${hlslib_flags}"
add_files     ${srcDir}/xf_harris_accel.cpp -cflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls ${VISION_INC_FLAGS} ${vitis_flags}" -csimflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls ${VISION_INC_FLAGS} ${vitis_flags}"
add_files -tb ${testDir}/test_${projectName}.cpp -cflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls ${VISION_INC_FLAGS} ${vitis_flags}" -csimflags "-I$env(cFpRootDir)/cFDK/SRA/LIB/hls ${VISION_INC_FLAGS} ${vitis_flags}"

# ------------------------------------------------------------------------------
# Create a solution
# ------------------------------------------------------------------------------
open_solution ${solutionName}

set_part      ${xilPartName}
create_clock -period 6.4 -name default

# ------------------------------------------------------------------------------
# Run C Simulation and Synthesis
# ------------------------------------------------------------------------------
if { $hlsSim } {
  csim_design -ldflags "${OPENCV_LIB_FLAGS} ${OPENCV_LIB_REF}" -clean -argv "${SimFile}"
} else {

  if { $hlsSyn } {
    csynth_design
  }
  
  if { $hlsCoSim } {
    cosim_design -ldflags "${OPENCV_LIB_FLAGS} ${OPENCV_LIB_REF}" -trace_level all -argv "${SimFile}"
  } else {

  # ------------------------------------------------------------------------------
  # Export RTL
  # ------------------------------------------------------------------------------
    export_design -rtl vhdl -format ${ipPkgFormat} -library ${ipLibrary} -display_name ${ipDisplayName} -description ${ipDescription} -vendor ${ipVendor} -version ${ipVersion}
  }
}

# ------------------------------------------------------------------------------
# Exit Vivado HLS
# ------------------------------------------------------------------------------
exit
