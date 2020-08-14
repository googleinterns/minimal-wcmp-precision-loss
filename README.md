# minimal-wcmp-precision-loss

## Problem Introduction

The WCMP precision loss problem came from the incompatible output format of the linear programming (LP) solver and the WCMP input. Currently, controller takes the traffic demand, network topology (Heterogeneous in terms of the bandwidth and link count), and the traffic demand uncertainty settings as the input, and the LP solver will try to minimize the maximum link bandwidth (MLU) and then give the weights for all next hops as the output. However, this weight assignment does not take the hardware constraints that we mentioned before into consideration, meaning that it may contain some weight assignment that cannot be supported by the switch hardware.

Though the precision loss on a single switch is bounded and predictable, and we can optimize the weight reduction algorithm, which is used to change the weights to suit the hardware constraints, to have less precision loss. But due to the topology and the routing policy in the network, this bounded precision loss will be amplified along with the transmission in the network and arrive at unbounded precision loss.

## Contents
This repository provided a platform for researchers to quantify the precision loss when using their own solvers, algorithms with a certain network topology and traffic pattern. 

The repo now provide four kinds of solvers to use: Path-based LP solver, Arc-based LP solver, Path-based ILP Solver and Arc-based ILP solver. Each of those solvers can be used with any arbitrary network topology and traffic matrix, the optimization goal of the solvers is to minimize the maximum link utilization and give the WCMP weight assignment. 

The topology folder provide three kinds of network topologies, the DCN only topology, full data center network topology and intra-domain only topology. Users is able to add their own topologies for evaluation. 

The trace folder provide 5 kinds of traffic pattern, and users are also encouraged to use the real traffic trace recorded with protobuf. The five traffic trace includes Random, Sparse, Permutation, Stride, and Symmetric.   

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

To change the solver and algorithm, for the solvers predefined in the repo, you can change the configurations in the wcmp/definitions.h. As for your own solvers, you may need to add it directly in the wcmp/main.cpp file.

To change the network topology to your own, you need to follow the style of the current topology, and then create your own network topology. Then specify the topology class that you will used in the wcmp/main.cpp file.

To use your own trace, you can import your own trace by simply calling the function Trace::ImportTrafficTrace(std::string {your_file_name}). 
	
## Note

This is not an official Google product.
