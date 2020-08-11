# minimal-wcmp-precision-loss

The project use the Linear Programming model to solve the ECMP group precision
loss problem under the hardware limitation. The input is the traffic demand and 
the network topology, and the output is the traffic assignment on each possible
paths. 

## Prerequisites

[Bliss patch](https://www.scipopt.org/download/bugfixes/scip-7.0.1/bliss-0.73.patch)

[SCIP](https://scipopt.org/index.php#download)

[ProtoBuf](https://developers.google.com/protocol-buffers/docs/downloads)

[GLOG](https://github.com/google/glog/blob/master/cmake/INSTALL.md)

To add the SCIP to your cmake environment: 

    cmake -Bbuild -H. [-DSOPLEX_DIR=/path/to/soplex]
    cmake --build build
    mkdir build
    cd build
    cmake .. [-DSOPLEX_DIR=/path/to/soplex]
    make
    # optional: run a quick check on some instances
    make check
    # optional: install scip executable, library, and headers
    make install
    cmake <path/to/SCIP> -LH

## Instructions 

To compile and run the project, simply follow the instructions:
    
    git clone git@github.com:googleinterns/minimal-wcmp-precision-loss.git
    cd minimal-wcmp-precision-loss/
    cd wcmp/
    mkdir build
    cd build/
    cmake ..
    make
    ./wcmp

Note: you may need to specify SCIP installation directory for cmake to find it:
export CMAKE_PREFIX_PATH=/<your installation directory>/SCIPOptSuite-7.0.1-Darwin/
    
## Development

The current implementation of network topology can be changed in the file 
*definitions.h*. And the customized trace can be import with the import function 
in the Trace class.
	
## Note

This is not an official Google product.