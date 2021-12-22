# *****************************************************************************
# *                            cloudFPGA
# *                Copyright 2016 -- 2022 IBM Corporation
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *     http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *----------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8)

project( cFpUppercase )

find_package(LibXml2 REQUIRED)
find_package(SWIG REQUIRED)

#set(PYTHON_INCLUDE_PATH /usr/include/python3.5m)
#set(PYTHON_LIBRARIES /usr/lib/x86_64-linux-gnu/libpython3.5m.so)
#set(PYTHON_EXECUTABLE /usr/bin/python3.5m)
#set(PYTHON_INCLUDE_DIR  /usr/include/python3.5m)
#set(Python_ADDITIONAL_VERSIONS 3.5)
#find_package(PythonInterp 3 REQUIRED)
set(PYTHON_LIBRARIES /usr/lib64/libpython3.8.so)
set (CMAKE_MODULE_PATH "$ENV{cFpRootDir}/HOST/custom/uppercase/languages/python/")
find_package(PythonLibs 3 REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS})
include(${SWIG_USE_FILE})

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()

set (CMAKE_CXX_FLAGS "-g -Wall -fstack-check -std=c++11 -DPY_WRAP")

add_executable( uppercase_host_fwd_tb ../cplusplus/src/uppercase_host_fwd_tb.cpp ../../../../PracticalSockets/src/PracticalSockets.cpp )
target_link_libraries( uppercase_host_fwd_tb ${LIBXML2_LIBRARIES} )

#this sets the output directory of anything SWIG generates
set (CMAKE_SWIG_OUTDIR  ./swig_trieres_output)

set_property(SOURCE ./trieres.i PROPERTY CPLUSPLUS ON)
swig_add_module(trieres python ./trieres.i ../../../../PracticalSockets/src/PracticalSockets.cpp ../cplusplus/src/uppercase_host.cpp)
swig_link_libraries (trieres ${PYTHON_LIBRARIES})
