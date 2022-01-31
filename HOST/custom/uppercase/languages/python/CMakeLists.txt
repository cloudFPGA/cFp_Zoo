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

find_package(PythonLibs 3 REQUIRED)
set (CMAKE_MODULE_PATH "$ENV{cFpRootDir}/HOST/vision/median_blur/languages/python/")

include_directories(${LIBXML2_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS})
include(${SWIG_USE_FILE})

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()



set(SOURCES_HOST_FWD_TB ../cplusplus/src/uppercase_host_fwd_tb.cpp
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")


set(SOURCES_HOST        ../cplusplus/src/uppercase_host.cpp
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")

set(INCLUDES_HOST       "$ENV{cFpRootDir}HOST/common/langs/cpp/include/"
                        "$ENV{cFpRootDir}HOST/custom/uppercase/languages/cplusplus/include/"
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/")


#this sets the output directory of anything SWIG generates
set (CMAKE_SWIG_OUTDIR  ./swig_trieres_output)

#set_source_files_properties (./trieres_median_blur_numpi.i PROPERTIES SWIG_FLAGS "-threads")

set (CMAKE_CXX_FLAGS "-ggdb3 -Wall -fstack-check -std=c++11 -DPY_WRAP")

set_property(SOURCE ./trieres.i PROPERTY CPLUSPLUS ON)
swig_add_module(trieres python ./trieres.i ${SOURCES_HOST})
include_directories(${INCLUDES_HOST})
#set_property(TARGET trieres_median_blur_numpi PROPERTY SWIG_USE_LIBRARY_INCLUDE_DIRECTORIES TRUE)

swig_link_libraries (trieres ${PYTHON_LIBRARIES})

add_executable( uppercase_host_fwd_tb ${SOURCES_HOST_FWD_TB} )
target_include_directories(uppercase_host_fwd_tb PRIVATE ${INCLUDES_HOST})
target_link_libraries( uppercase_host_fwd_tb ${LIBXML2_LIBRARIES} )

