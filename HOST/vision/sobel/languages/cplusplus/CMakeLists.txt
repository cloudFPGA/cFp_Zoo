cmake_minimum_required(VERSION 2.8)
project( cFpvitis_Vision_Sobel )
find_package( OpenCV REQUIRED )
find_package( LibXml2 REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR})


get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()

set (CMAKE_CXX_FLAGS "-ggdb3 -Wall -fstack-check -std=c++11")

set(SOURCES_HOST_FWD_TB src/sobel_host_fwd_tb.cpp
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/util.cpp"
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/ologger.cpp"
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")


set(SOURCES_HOST        src/sobel_host.cpp
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/util.cpp"
                        "$ENV{cFpRootDir}HOST/common/langs/cpp/src/ologger.cpp"                        
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/PracticalSockets.cpp")

set(INCLUDES_HOST       "$ENV{cFpRootDir}HOST/common/langs/cpp/include/"
                        "$ENV{cFpRootDir}HOST/vision/sobel/languages/cplusplus/include/"
                        "$ENV{cFpRootDir}HOST/PracticalSockets/src/")

add_executable( sobel_host_fwd_tb ${SOURCES_HOST_FWD_TB} )
target_include_directories(sobel_host_fwd_tb PRIVATE ${INCLUDES_HOST})
target_link_libraries( sobel_host_fwd_tb ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )

add_executable( sobel_host ${SOURCES_HOST} )
target_include_directories(sobel_host PRIVATE ${INCLUDES_HOST})
target_link_libraries( sobel_host ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )

