cmake_minimum_required(VERSION 3.9.0)
project(motion_detector_driver VERSION 1.0.0 DESCRIPTION "Driver program for motion detector library")

set(DEFAULT_BUILD_TYPE "Release")

# Find the OpenCV package in the system
find_package(OpenCV REQUIRED)

# Add the main.cpp to the executable file
add_executable(${PROJECT_NAME} main.cpp)

# Link OpenCV to the executable
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# Link motion_detector library to the executable
target_link_libraries(${PROJECT_NAME} motion_detector)

# Link the filesystem library
target_link_libraries(${PROJECT_NAME} stdc++fs)

# Set compiler flags. Tell it to treat warnings as errors, pedantic and c++11
target_compile_options(${PROJECT_NAME} PRIVATE -Werror -pedantic)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)


