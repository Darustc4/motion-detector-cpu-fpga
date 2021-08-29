# Motion Detector for CPU & FPGA HLS
Simple motion detector library programmed in modern C++ with no dependencies. Capable of processing Full HD video in real time with CPU only.

# CPU

The CPU version, if the correct configuration is selected and the host CPU is powerful enough, can reach real time processing at 30 FPS.

There is 2 versions of the library, a "base" library which is generic and reusable, and a "fast" library which is highly specialized and works magnitudes faster.

This library supports a variety of driver programs that make use of it, 2 such examples can be found in this repository, one which saves video to disk when motion is detected in the selected folder, and another which shows the motion detected on screen constantly.

Note that even if the library has no dependencies at all, the pilot programs depend on OpenCV to interact with the camera or open .mp4 files for simplicity.

# FPGA

The base library for CPU has been adapted step by step to an FPGA IP core using Vitis HLS. This port is meant to be didactic and simple, showing the main key points to take into account when modern C++ needs to be adapted to hardware which requires much lower abstraction levels.

The resulting optimized code can reach up to 190 FPS when there is no movement in the feed and, in the worst case possible being extremely complex movement or noise, 13 FPS. On average when there is controlled movement in the feed such as people walking or objects being moved around, the FPS remains above 100.

The hardware resources used are low, at 146 18K BRAMs, 19 DSPs, 9957, FFs and 11335 LUTs, although this could be improved further with more careful use of HLS pragmas and code refactoring. 

In order to test the code, a testbench is given which read a .mp4 video from disk and feeds it to the IP core.