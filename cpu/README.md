# Compiling and using the CPU version

Important notice: The CMake file for this library only contemplates GNU/Linux package installation. The library can still be compiled on Windows, but the library will NOT be installed.
 
## Compiling, installing and testing the motion detector library.

These instructions assume this repo was downloaded directly into the home directory of a user named "md", on a Raspberry OS based system named "pi". The repo name is "motdet", and the base library will be compiled, in case you want to compile and install the fast library instead, simply change the path in the instructions below from /base/ to /fast/.

```console
md@pi:~ $ sudo apt update -y && sudo apt upgrade -y
md@pi:~ $ sudo apt install -y build-essential cmake pkg-config libgtk-3-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev gfortran openexr libatlas-base-dev python3-dev python3-numpy libtbb2 libtbb-dev libdc1394-22-dev libavutil-dev libavresample-dev libsdl1.2-dev libjack-jackd2-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libva-dev libvdpau-dev libvorbis-dev libx11-dev libxfixes-dev texi2html zlib1g-dev libvpx-dev libgtk2.0-dev libqt4-dev libqt4-opengl-dev libtiff5-dev libjasper-dev libpng12-dev v4l-utils x264 yasm
md@pi:~ $ sudo apt install -y clang --install-suggests
md@pi:~ $ cd ~/motdet/cpu/libraries/base
```

Now that all the necessary packages are installed, let's build the actual library. Set DBUILD_TEST to false if you do not want to compile the unit tests.

```console
md@pi:~/motdet/cpu/libraries/base $ mkdir build && cd build
md@pi:~/motdet/cpu/libraries/base $ export CXX=/usr/bin/clang++
md@pi:~/motdet/cpu/libraries/base/build $ cmake .. -DBUILD_TEST=true
md@pi:~/motdet/cpu/libraries/base/build $ sudo make install -j4
```

If you compiled the tests, run them with the following commands:

```console
md@pi:~/motdet/cpu/libraries/base/build $ pkg-config --modversion motion_detector
md@pi:~/motdet/cpu/libraries/base/build $ ./test_exec
```

## Compiling and running an example driver program.

The example save_to_disk driver program that will be compiled here reads frames from a camera connected to the device or from a .mp4 file.
In order to read and save the detected data in this driver program we use OpenCV for simplicity, but note that the motion detector library itself does not use OpenCV to work.
If you wish to use this driver program for testing, please install OpenCV with support for C++ for your distribution before continuing with the following commands:

```console
md@pi:~ $ mkdir ~/opencv && cd ~/opencv
md@pi:~/opencv $ sudo apt install -y git
md@pi:~/opencv $ git clone https://github.com/opencv/opencv.git
md@pi:~/opencv $ git clone https://github.com/opencv/opencv_contrib.git
md@pi:~/opencv $ cd opencv && mkdir build && cd build
md@pi:~/opencv/opencv/build $ cmake .. -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D INSTALL_C_EXAMPLES=OFF -D INSTALL_PYTHON_EXAMPLES=OFF -D OPENCV_GENERATE_PKGCONFIG=ON -D OPENCV_EXTRA_MODULES_PATH=~/opencv/opencv_contrib/modules -D BUILD_EXAMPLES=OFF ..
md@pi:~/opencv/opencv/build $ sudo make install -j4
```

Once we have OpenCV installed, let's compile the driver program:
```console
md@pi:~ $ cd ~/motdet/cpu/pilot_programs/save_to_disk
md@pi:~/motdet/cpu/pilot_programs/save_to_disk $ mkdir build && cd build
md@pi:~/motdet/cpu/pilot_programs/save_to_disk $ export CXX=/usr/bin/clang++
md@pi:~/motdet/cpu/pilot_programs/save_to_disk/build $ cmake ..
md@pi:~/motdet/cpu/pilot_programs/save_to_disk/build $ make -j4
```
The executable "motion_detector_driver" has been created.
If we try to execute it will say not enough arguments have been passed.

The arguments that this executable receives are the following:
 * input: If we select "camera" it will read frames from the camera, if instead a path to a .mp4 file is given, it will read it instead.
 * output: Path to a directory. When motion is detected, a new video will be recorded to this directory with the motion detected closed within a green box.
 * [Optional] threads : Integer for the amount of threads to use for processing, must be >0. Default is 1.
 * [Optional] downscale factor: Integer for the reduction of resolution to be applied to processed frames, the higher the faster, but after 4 it will lose too much detail to work properly (Assuming 1080p input, for lower resolutions this factor will be lower). Must be > 0. Default is 1 (quite slow).
 * [Optional] display stats: 1 for true, 0 for false. Will print out the times the detector took for each processing step.
 
 A possible effective running command would be:
 ```console
 md@pi:~/motdet/cpu/pilot_programs/save_to_disk/build $ ./motion_detector_driver ~/motdet/example_results/in_test_motion.mp4 ~/motdet/example_results 4 4 1 
 ```
 