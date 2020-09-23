cmake_minimum_required(VERSION 2.8) 
project( colorWheelHSV C CXX )

set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
set(CMAKE_C_FLAGS "-Wall -std=gnu99 -fno-omit-frame-pointer -funwind-tables") 
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -fno-omit-frame-pointer -funwind-tables") 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DON_LINUX -D__STDC_CONSTANT_MACROS)

find_package( OpenCV 2.4 REQUIRED )
find_package(CUDA)

add_executable( colorWheelHSV  src/ColorWheelHSV.cpp )
target_link_libraries( colorWheelHSV ${OpenCV_LIBS} )
