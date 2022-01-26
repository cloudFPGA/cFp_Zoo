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
project( cFpvitis_Vision_WarpTransform )
find_package( OpenCV REQUIRED )
find_package( LibXml2 REQUIRED)
find_package(Boost COMPONENTS system filesystem)

include_directories(${LIBXML2_INCLUDE_DIR})


get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()

set (CMAKE_CXX_FLAGS "-ggdb3 -Wall -fstack-check -std=c++11")

set(SOURCES_HOST_FWD_TB src/warp_transform_host_fwd_tb.cpp
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/util.cpp"
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/ologger.cpp"
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")


set(SOURCES_HOST        src/warp_transform_host.cpp
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/util.cpp"
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/ologger.cpp"                        
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")

set(INCLUDES_HOST       "$ENV{cFpRootDir}HOST/common/langs/cpp/include/"
                        "$ENV{cFpRootDir}HOST/vision/warp_transform/languages/cplusplus/include/"
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/")

set(SOURCES_HOST_LIGHT  src/warp_transform_host_lightweight.cpp
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/util.cpp"
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/ologger.cpp"                        
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")

set(INCLUDES_HOST_LIGHT "$ENV{cFpRootDir}HOST/common/langs/cpp/include/"
                        "$ENV{cFpRootDir}HOST/vision/warp_transform/languages/cplusplus/include/"
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/")

set(SOURCES_HOST_SW_PARALLEL        src/warp_transform_host_parallel.cpp
                      "$ENV{cFpRootDir}HOST/common/langs/cpp/src/util.cpp"
                      "$ENV{cFpRootDir}HOST/common/langs/cpp/src/ologger.cpp"
                      "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")


set(INCLUDES_SW_PARALLEL    "$ENV{cFpRootDir}HOST/common/langs/cpp/include/"
                        "$ENV{cFpRootDir}HOST/vision/warp_transform/languages/cplusplus/include/"
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/")

add_executable( warp_transform_host_fwd_tb ${SOURCES_HOST_FWD_TB} )
target_include_directories(warp_transform_host_fwd_tb PRIVATE ${INCLUDES_HOST})
target_link_libraries( warp_transform_host_fwd_tb ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )

add_executable( warp_transform_host ${SOURCES_HOST} )
target_include_directories(warp_transform_host PRIVATE ${INCLUDES_HOST})
target_link_libraries( warp_transform_host ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )

add_executable( warp_transform_host_lightweight ${SOURCES_HOST_LIGHT} )
target_include_directories(warp_transform_host_lightweight PRIVATE ${INCLUDES_HOST_LIGHT})
target_link_libraries( warp_transform_host_lightweight ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )

set(CMAKE_CXX_FLAGS "-ggdb3 -fstack-check -Wall -std=c++11 -fopenmp -lboost_system -lboost_filesystem")
add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
add_definitions(-DBOOST_SYSTEM_NO_DEPRECATED)
add_executable( warp_transform_host_parallel ${SOURCES_HOST_SW_PARALLEL} )
target_include_directories(warp_transform_host_parallel PRIVATE ${INCLUDES_SW_PARALLEL})
target_link_libraries( warp_transform_host_parallel ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} ${Boost_LIBRARIES})
