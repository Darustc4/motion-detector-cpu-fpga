# HLS code

This project puts it's emphasis on the process of transforming modern C++ code that makes heavy use of the STL and threading so that it can be used in a FPGA.
The different steps followed and the methodology can be followed in the different directories numerated here.

* 0 - Adapting to HLS. Step by step removing and adapting all the code that is not compatible with HLS.
    * HLS limitations:
        * System calls. Threads must be eliminated. No standard or file I/O. No time operations such as sleep.
        * Dynamic memory. Cannot use malloc or new, meaning the STL must go (vector, array, unique_ptr, deque, and many more).
        * Generic pointers. Cannot use void* or shared_ptr<void>.
        * Function pointers. Passing a function as a paramenter uses function pointers, it cannot be done.
        * Recursive functions. Thankfully the motion detector library has none of these.
        * Other minor restrictions. For a full in depth list, see the HLS documentation.
    * This first step in converting modern C++ to HLS ends when the code can be synthetized for the first time and work (albeit extremely slow).
    
* 1 - HLS optimization. Step by step optimizing and streamlining the code with HLS directives and re-estructuring.
    * Steps to follow:
        * Initial optimizations. Data interfaces, data packing and loop tripcounts.
        * Pipelines. Define pipelines, dataflow areas, loop unrolling...
        * Structure optimization. Partition memories and ports, and remove false dependencies.
        * Latency reduction. Specify latency requirements.
        * Area reduction. Share resources to reduce the amount of components used by the IP core.

* 2 - Final HLS code.