cmake_minimum_required(VERSION 3.9.0)
project(motion_detector VERSION 1.0.0 DESCRIPTION "No dependency motion detector library in C++")

set(DEFAULT_BUILD_TYPE "Release")

set(SOURCE_FILES src/motion_detector.cpp src/contour_detector.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

# Set library version
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

# Set version of the generated so files (For example: libmotion_detector.so.1.0.0.)
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION 1)

# Avoid having to include with relative paths
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set the file with the public API for the library
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER include/motion_detector.hpp)

# Set compiler flags. Tell it to treat warnings as errors, pedantic and c++11
target_compile_options(${PROJECT_NAME} PRIVATE -Werror -pedantic)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries (${PROJECT_NAME} LINK_PUBLIC pthread)

if(BUILD_TEST)
    set(TEST_FILES test/test_motion_detector.cpp test/test_contour_detector.cpp test/test_image_utils.cpp)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

    # Compile the test sources while linking to the main library
    add_library (test_lib ${TEST_FILES})
    target_link_libraries (test_lib LINK_PUBLIC ${PROJECT_NAME})

    target_include_directories(test_lib
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_compile_options(test_lib PRIVATE -Werror -pedantic)
    target_compile_features(test_lib PRIVATE cxx_std_17)

    # Compile the main test executable while linking to the test library
    add_executable (test_exec test/test_main.cpp)
    target_link_libraries (test_exec LINK_PUBLIC test_lib)

    target_include_directories(test_exec
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_compile_options(test_exec PRIVATE -Werror -pedantic)
    target_compile_features(test_exec PRIVATE cxx_std_17)
endif()