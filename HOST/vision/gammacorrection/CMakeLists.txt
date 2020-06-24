cmake_minimum_required(VERSION 2.8)
project( cFpvitis_Vision_Gammacorrection )
find_package( OpenCV REQUIRED )
find_package( LibXml2 REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR})

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()

set (CMAKE_CXX_FLAGS "-g -Wall -fstack-check -std=c++11")

add_executable( gammacorrection_host_fwd_tb src/gammacorrection_host_fwd_tb.cpp ../../PracticalSockets/src/PracticalSockets.cpp )
target_link_libraries( gammacorrection_host_fwd_tb ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )
add_executable( gammacorrection_host src/gammacorrection_host.cpp ../../PracticalSockets/src/PracticalSockets.cpp )
target_link_libraries( gammacorrection_host ${OpenCV_LIBS} ${LIBXML2_LIBRARIES} )

