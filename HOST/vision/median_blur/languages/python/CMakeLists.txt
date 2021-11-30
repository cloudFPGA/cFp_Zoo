cmake_minimum_required(VERSION 2.8)

project( cFpvitis_Vision_MedianBlur )
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
set (CMAKE_MODULE_PATH "$ENV{cFpRootDir}/HOST/vision/median_blur/languages/python/")

include_directories(${LIBXML2_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS})
include(${SWIG_USE_FILE})

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()



set(SOURCES_HOST_FWD_TB ../cplusplus/src/median_blur_host_fwd_tb.cpp
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/util.cpp"
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/ologger.cpp"
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")


set(SOURCES_HOST        ../cplusplus/src/median_blur_host.cpp
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/util.cpp"
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/ologger.cpp"        
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")

set(INCLUDES_HOST       "$ENV{cFpRootDir}HOST/common/langs/cpp/include/"
                        "$ENV{cFpRootDir}HOST/vision/median_blur/languages/cplusplus/include/"
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/")


#this sets the output directory of anything SWIG generates
set (CMAKE_SWIG_OUTDIR  ./swig_trieres_output)

add_custom_target(swignumpi ALL
  COMMAND  "../etc/checknumpi.sh"
)

#set_source_files_properties (./trieres_median_blur_numpi.i PROPERTIES SWIG_FLAGS "-threads")

set (CMAKE_CXX_FLAGS "-ggdb3 -Wall -fstack-check -std=c++11 -DPY_WRAP=PY_WRAP_MEDIANBLUR_NUMPI")

set_property(SOURCE ./trieres_median_blur_numpi.i PROPERTY CPLUSPLUS ON)
swig_add_module(trieres_median_blur_numpi python ./trieres_median_blur_numpi.i ${SOURCES_HOST})
include_directories(${INCLUDES_HOST})
#set_property(TARGET trieres_median_blur_numpi PROPERTY SWIG_USE_LIBRARY_INCLUDE_DIRECTORIES TRUE)

swig_link_libraries (trieres_median_blur_numpi ${OpenCV_LIBS} ${PYTHON_LIBRARIES})

add_executable( median_blur_host_fwd_tb ${SOURCES_HOST_FWD_TB} )
target_include_directories(median_blur_host_fwd_tb PRIVATE ${INCLUDES_HOST})
target_link_libraries( median_blur_host_fwd_tb ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )

