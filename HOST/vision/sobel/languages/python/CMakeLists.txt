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

project( cFpvitis_Vision_Sobel )
find_package(LibXml2 REQUIRED)
find_package(SWIG REQUIRED)
find_package(OpenCV REQUIRED)

#set(PYTHON_INCLUDE_PATH /usr/include/python3.5m)
#set(PYTHON_LIBRARIES /usr/lib/x86_64-linux-gnu/libpython3.5m.so)
#set(PYTHON_LIBRARIES /usr/lib64/libpython3.8.so)
#set(PYTHON_EXECUTABLE /usr/bin/python3.5m)
#set(PYTHON_INCLUDE_DIR  /usr/include/python3.5m)
#set(Python_ADDITIONAL_VERSIONS 3.5)
#find_package(PythonInterp 3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)
set (CMAKE_MODULE_PATH "$ENV{cFpRootDir}/HOST/vision/sobel/languages/python/")

include_directories(${LIBXML2_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS})
include(${SWIG_USE_FILE})

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()


#this sets the output directory of anything SWIG generates
set (CMAKE_SWIG_OUTDIR  ./swig_trieres_output)

add_custom_target(swignumpi ALL
  COMMAND  "../etc/checknumpi.sh"
)

#set (CMAKE_CXX_FLAGS "-g -Wall -fstack-check -std=c++11 -DPY_WRAP=PY_WRAP_SOBEL_FILENAME")
#set_property(SOURCE ./trieres_sobel_filename.i PROPERTY CPLUSPLUS ON)
#swig_add_module(trieres_sobel_filename python ./trieres_sobel_filename.i ../../../../PracticalSockets/src/PracticalSockets.cpp ../cplusplus/src/sobel_host.cpp)
#swig_link_libraries (trieres_sobel_filename ${OpenCV_LIBS} ${PYTHON_LIBRARIES})

#set_source_files_properties (./trieres_sobel_numpi.i PROPERTIES SWIG_FLAGS "-threads")

set (CMAKE_CXX_FLAGS "-g -Wall -fstack-check -std=c++11 -DPY_WRAP=PY_WRAP_SOBEL_NUMPI")
set_property(SOURCE ./trieres_sobel_numpi.i PROPERTY CPLUSPLUS ON)
swig_add_module(trieres_sobel_numpi python ./trieres_sobel_numpi.i ../../../../PracticalSockets/src/PracticalSockets.cpp ../cplusplus/src/sobel_host.cpp)
swig_link_libraries (trieres_sobel_numpi ${OpenCV_LIBS} ${PYTHON_LIBRARIES})

add_executable( sobel_host_fwd_tb ../cplusplus/src/sobel_host_fwd_tb.cpp ../../../../PracticalSockets/src/PracticalSockets.cpp )
target_link_libraries( sobel_host_fwd_tb  ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )

