cmake_minimum_required(VERSION 2.8)
project( cFpvitis_Vision_Uppercase )
find_package( LibXml2 REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR})

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Included dir='${dir}'")
endforeach()

set (CMAKE_CXX_FLAGS "-g -Wall -fstack-check -std=c++11")

add_executable( uppercase_host_fwd_tb ./src/uppercase_host_fwd_tb.cpp ../../PracticalSockets/src/PracticalSockets.cpp )
target_link_libraries( uppercase_host_fwd_tb ${LIBXML2_LIBRARIES} )
add_executable( uppercase_host ./src/uppercase_host.cpp ../../PracticalSockets/src/PracticalSockets.cpp )
target_link_libraries( uppercase_host ${LIBXML2_LIBRARIES} )
