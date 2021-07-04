# ------------------------------------------------------------------------------
# Vitis Vision and OpenCV Libary Path Information
# ------------------------------------------------------------------------------

set OPENCV_INCLUDE "E:/Programs/OpenCV4/include"
set OPENCV_LIB "E:/Programs/OpenCV4/x64/mingw/lib"

# ------------------------------------------------------------------------------
# Vitis HLS Project Information
# ------------------------------------------------------------------------------
set PROJ_DIR "C:/Users/bfant/Desktop/Work/University/TFG/motion_detector/repo/motion-detector-cpu-fpga/hls/1_hls_optimization/0_streaming_images"
set SOURCE_DIR "$PROJ_DIR/src"
set TB_DIR "$PROJ_DIR/testbench"
set PROJ_NAME "motdet_hls"
set PROJ_TOP "detect_motion"
set SOLUTION_NAME "initial"
set SOLUTION_PART "xc7z020clg400-1"
set SOLUTION_CLKP 10

# ------------------------------------------------------------------------------
# OpenCV C Simulation / CoSimulation Library References
#------------------------------------------------------------------------------
set OPENCV_INC_FLAGS "-I$OPENCV_INCLUDE"
set OPENCV_LIB_FLAGS "-L$OPENCV_LIB"

# Windows OpenCV Include Style. THE trailing number is the OpenCV version installed, change accordingly.
set OPENCV_LIB_REF "-lopencv_videoio452 -lopencv_video452 -lopencv_imgproc452 -lopencv_imgcodecs452 -lopencv_core452"

# ------------------------------------------------------------------------------
# Create Project
# ------------------------------------------------------------------------------
open_project -reset $PROJ_NAME

# ------------------------------------------------------------------------------
# Add C++ source and Testbench files with Vision and OpenCV includes
# ------------------------------------------------------------------------------
add_files "${SOURCE_DIR}/motion_detector.hpp"
add_files "${SOURCE_DIR}/motion_detector.cpp"
add_files "${SOURCE_DIR}/image_utils.hpp"
add_files "${SOURCE_DIR}/image_utils.cpp"
add_files "${SOURCE_DIR}/contour_detector.hpp"
add_files "${SOURCE_DIR}/contour_detector.cpp"
add_files -tb "${TB_DIR}/main.cpp" -cflags "${OPENCV_INC_FLAGS}" -csimflags "${OPENCV_INC_FLAGS}"

# ------------------------------------------------------------------------------
# Create Project and Solution
# ------------------------------------------------------------------------------
set_top $PROJ_TOP
open_solution -reset $SOLUTION_NAME
set_part $SOLUTION_PART
create_clock -period $SOLUTION_CLKP

# ------------------------------------------------------------------------------
# Run Vitis HLS Stages
# Note: CSim and CoSim require datafiles to be included
# ------------------------------------------------------------------------------

# C simulation
csim_design -ldflags "${OPENCV_LIB_FLAGS} ${OPENCV_LIB_REF}"

# Synthesis
#csynth_design

# Simulate with synthesized, and export if it works
#cosim_design -ldflags "${OPENCV_LIB_FLAGS} ${OPENCV_LIB_REF}"
#export_design -flow syn -rtl vhdl
#export_design -flow impl -rtl vhdl

exit
