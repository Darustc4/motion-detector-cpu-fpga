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

if (UNIX)
    # To install a library in linux it is required to install both the .so files in /lib and the headers in /include
    # Once we do this, the library can be used (shared) by any other project run on the system, very convenient.
    # To make this even more convenient to use, we will also create a package so you can do -lmotion_detector when using g++

    # Define GNU standard installation directories
    include(GNUInstallDirs)

    # Install libs and includes
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    # Create package so that it can included easily by other projects. This requires a file named "motion_detector.pc.in" to exist
    # All the package info will be stored in there.
    configure_file(motion_detector.pc.in motion_detector.pc @ONLY)
    install(FILES ${CMAKE_BINARY_DIR}/motion_detector.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
else()
    message(STATUS ">>> Not UNIX/Linux OS detected, installing motion_detector automatically is not contemplated for this OS, please install the generated files as needed.")
endif()
