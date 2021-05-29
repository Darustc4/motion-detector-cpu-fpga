# Compiling and using the CPU version

Important notice: The CMake file for this library only contemplates GNU/Linux package installation. The library can still be compiled on Windows, but the library will NOT be installed.

Commands 

## Compiling, installing and testing the motion detector library.

Open a terminal/command line and navigate to the directory where this README is located.
```console
daru@uca:~/source/cpu$ cd motion_detector; mkdir build; cd build
daru@uca:~/source/cpu/motion_detector/build$ cmake .. -DBUILD_TEST=true; make -j8; sudo make install
daru@uca:~/source/cpu/motion_detector/build$ ./test_exec
```
If all the tests PASSED, the library is ready to be used by other programs. Go back to the
```console
daru@uca:~/source/cpu/motion_detector/build$ cd ../..
```

## Compiling and running the driver program (main).

This driver program reads frames from a camera connected to the device or from a .mp4 file.
In order to read and save the detected data in this driver program we use OpenCV for simplicity, but note that the motion detector library itself does not use OpenCV to work.
If you wish to use this driver program for testing, please install OpenCV with support for C++ for your distribution before continuing.

Once we have OpenCV installed, let's compile the driver program:
```console
daru@uca:~/source/cpu$ cd main; mkdir build; cd build
daru@uca:~/source/cpu/main/build$ cmake ..; make
```
The executable "motion_detector_driver" has been created.
If we try to execute it will say not enough arguments have been passed.

The arguments that this executable receives are the following:
 * input: If we select "camera" it will read frames from the camera, if instead a path to a .mp4 file is given, it will read it instead.
 * output: Path to a directory. When motion is detected, a new video will be recorded to this directory with the motion detected closed within a green box.
 * [Optional] threads : Integer for the amount of threads to use for processing, must be >0. Default is 1.
 * [Optional] downscale factor: Integer for the reduction of resolution to be applied to processed frames, the higher the faster, but after 4 it will lose too much detail to work properly (Assuming 1080p input, for lower resolutions this factor will be lower). Must be > 0. Default is 1 (quite slow).
 * [Optional] display stats: 1 for true, 0 for false. Will print out the times the detector took for each processing step.