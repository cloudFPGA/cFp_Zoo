cmake_minimum_required(VERSION 2.8)
project( lan_vid_pseudostream )
find_package( OpenCV REQUIRED )
find_package( LibXml2 REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR})
include_directories($ENV{XILINX_ROOT}/Vivado/$ENV{XILINX_VERSION}/include)
include_directories($ENV{XILINX_ROOT}/Vitis/$ENV{XILINX_VERSION}/include)
include_directories($ENV{cFpRootDir}/Vitis_Libraries/vision/L1/include)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()

set (CMAKE_CXX_FLAGS "-g -Wall -fstack-check -Wno-reorder -Wno-unknown-pragmas -Wno-attributes -D__SDSVHLS__ -std=c++0x")

add_executable( harris_host_fwd_tb src/harris_host_fwd_tb.cpp src/PracticalSocket.cpp )
target_link_libraries( harris_host_fwd_tb ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )
add_executable( harris_host src/harris_host.cpp src/PracticalSocket.cpp )
target_link_libraries( harris_host ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )

