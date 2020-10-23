cmake_minimum_required(VERSION 2.8)

project( cFpMCEuropeanEngine )

find_package(LibXml2 REQUIRED)
find_package(SWIG REQUIRED)

set(PYTHON_INCLUDE_PATH /usr/include/python3.5m)
set(PYTHON_LIBRARIES /usr/lib/x86_64-linux-gnu/libpython3.5m.so)
set(PYTHON_EXECUTABLE /usr/bin/python3.5m)
set(PYTHON_INCLUDE_DIR  /usr/include/python3.5m)
set(Python_ADDITIONAL_VERSIONS 3.5)
find_package(PythonInterp 3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS})
include(${SWIG_USE_FILE})

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()

set (CMAKE_CXX_FLAGS "-g -Wall -fstack-check -std=c++11 -DPY_WRAP")

#this sets the output directory of anything SWIG generates
set (CMAKE_SWIG_OUTDIR  ./swig_trieres_output)

add_custom_target(swignumpi ALL
  COMMAND  "../etc/checknumpi.sh"
)

set_property(SOURCE ./trieres.i PROPERTY CPLUSPLUS ON)
swig_add_module(trieres python ./trieres.i ../../../../PracticalSockets/src/PracticalSockets.cpp ../cplusplus/src/mceuropeanengine_host.cpp)
swig_link_libraries (trieres ${PYTHON_LIBRARIES})

add_executable( mceuropeanengine_host_fwd_tb ../cplusplus/src/mceuropeanengine_host_fwd_tb.cpp ../../../../PracticalSockets/src/PracticalSockets.cpp )
target_link_libraries( mceuropeanengine_host_fwd_tb ${LIBXML2_LIBRARIES} )