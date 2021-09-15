cmake_minimum_required(VERSION 2.8)
project( cFpvitis_Vision_Memtest )
find_package( LibXml2 REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR})

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()

set (CMAKE_CXX_FLAGS "-g -Wall -fstack-check -std=c++11")

add_executable( memtest_host_fwd_tb ./src/memtest_host_fwd_tb.cpp ../../../../PracticalSockets/src/PracticalSockets.cpp )
target_link_libraries( memtest_host_fwd_tb ${LIBXML2_LIBRARIES} )
add_executable( memtest_host ./src/memtest_host.cpp ../../../../PracticalSockets/src/PracticalSockets.cpp )
target_link_libraries( memtest_host ${LIBXML2_LIBRARIES} )
