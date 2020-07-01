cmake_minimum_required(VERSION 3.10)

project(wcmp)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

find_package(SCIP REQUIRED)
find_package (glog REQUIRED)

add_executable(wcmp
		traffic_trace/Trace.cpp
		traffic_trace/Trace.h
#		components/Link.cpp
#		components/Link.h
#		components/Path.cpp
#		components/Path.h
#		components/Switch.cpp
#		components/Switch.h
#		topology/full/FullTopology.cpp
#		topology/full/FullTopology.h
		topology/dcnonly/DCNOnlyTopology.cpp
		topology/dcnonly/DCNOnlyTopology.h
		topology/DCN.h
		definitions.h
		main.cpp)

target_link_libraries(wcmp libscip glog::glog)
