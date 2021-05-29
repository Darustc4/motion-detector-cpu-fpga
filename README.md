# Motion Detector for CPU & FPGA HLS
Simple motion detector library programmed in modern C++ with no dependencies. All processing is done by the CPU.

There are 2 versions of the code, one that uses multithreading and makes heavy use of the STL, meant for CPU and another version with no multithreading and little to no use of the STL, meant to be used on a FPGA (using HLS).

# CPU

The CPU version, if the correct configuration is selected and the host CPU is powerful enough, can reach real time processing at near 30 FPS.
Note this library was never meant to be the fastest, it is a mere "example" library that will be adapted to FPGA using HLS.

# FPGA

TBD 