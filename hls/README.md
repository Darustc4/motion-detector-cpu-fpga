# The HLS IP core

Both the final version of the HLS IP core with it's source code and all the steps followed to adapt the modern C++ library to it are contained in this directory.

In order to synthetize this IP core, you will need VITIS HLS, and, if you also want to use the thestbench (main.cpp), you will need to link an OpenCV library to the project. .tcl files with all the necessary configuration are offered, but they need to be edited to point at a valid OpenCV installation path.

For details on why all the steps followed to adapt the library are here, and for an explanation of each step, please read the project .pdf at the root of the repo. NOTE: This documentation is in spanish only, and not translated. 