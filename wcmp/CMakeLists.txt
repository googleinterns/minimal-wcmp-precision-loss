cmake_minimum_required(VERSION 3.10)

project(wcmp)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

find_package(SCIP REQUIRED)
find_package (glog REQUIRED)
find_package (Protobuf REQUIRED)

add_executable(wcmp
		trace/Trace.cpp
		trace/Trace.h
		trace/trace.pb.cc
		trace/trace.pb.h
		topology/full/FullTopology.cpp
		topology/full/FullTopology.h
		topology/dcnonly/DCNOnlyTopology.cpp
		topology/dcnonly/DCNOnlyTopology.h
		topology/intradomain/IntraDomainTopology.cpp
		topology/intradomain/IntraDomainTopology.h
		topology/AbstractTopology.h
		solver/AbstractSolver.h
		solver/PathBasedLPSolver.cpp
		solver/PathBasedLPSolver.h
		solver/ArcBasedLPSolver.cpp
		solver/ArcBasedLPSolver.h
		solver/PathBasedILPSolver.cpp
		solver/PathBasedILPSolver.h
		solver/ArcBasedILPSolver.cpp
		solver/ArcBasedILPSolver.h
		solver/IntraDomainILPSolver.cpp
		solver/IntraDomainILPSolver.h
		definitions.h
		weightReduction.cpp
		main.cpp)

target_link_libraries(wcmp libscip glog::glog ${PROTOBUF_LIBRARY})
