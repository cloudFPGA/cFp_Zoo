cmake_minimum_required(VERSION 2.8)
project( cFpvitis_Vision_MedianBlur )
find_package( OpenCV REQUIRED )
find_package( LibXml2 REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR})


get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()

set (CMAKE_CXX_FLAGS "-ggdb3 -Wall -fstack-check -std=c++11")

set(SOURCES_HOST_FWD_TB src/median_blur_host_fwd_tb.cpp
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/util.cpp"
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/ologger.cpp"
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")


set(SOURCES_HOST        src/median_blur_host.cpp
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/util.cpp"
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/ologger.cpp"                        
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")

set(INCLUDES_HOST       "$ENV{cFpRootDir}HOST/common/langs/cpp/include/"
                        "$ENV{cFpRootDir}HOST/vision/median_blur/languages/cplusplus/include/"
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/")

add_executable( median_blur_host_fwd_tb ${SOURCES_HOST_FWD_TB} )
target_include_directories(median_blur_host_fwd_tb PRIVATE ${INCLUDES_HOST})
target_link_libraries( median_blur_host_fwd_tb ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )

add_executable( median_blur_host ${SOURCES_HOST} )
target_include_directories(median_blur_host PRIVATE ${INCLUDES_HOST})
target_link_libraries( median_blur_host ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )

