# minimal-wcmp-precision-loss

The project use the Linear Programming model to solve the ECMP group precision
loss problem under the hardware limitation. The input is the traffic demand and 
the network topology, and the output is the traffic assignment on each possible
paths. 

## Prerequisites

[SCIP](https://scipopt.org/index.php#download)

[ProtoBuf](https://developers.google.com/protocol-buffers/docs/downloads)

[GLOG](https://github.com/google/glog/blob/master/cmake/INSTALL.md)

## Instructions 

To compile and run the project, simply follow the instructions:

    mkdir build
    cd build
    cmake ..
    make
    ./wcmp
    
## Development

The current implementation of network topology can be changed in the file 
*definitions.h*. And the customized trace can be import with the import function 
in the Trace class.
	
## Note

This is not an official Google product.