//
// Created by wwt on 6/20/20.
//

#include <random>
#include <glog/logging.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Trace.h"
#include "trace.pb.h"
//#include "trace.pb.h"

namespace wcmp {
namespace traffic {

// set the traffic amount to be 10
SbTrafficMatrix GenerateSymmetricMatrix(int num_sb) {
  SbTrafficMatrix matrix = {};
  for (int src_sb = 0; src_sb < num_sb; ++src_sb) {
    for (int dst_sb = 0; dst_sb < num_sb; ++dst_sb) {
      matrix[src_sb * num_sb + dst_sb] = 10;
    }
  }
  return matrix;
}

// use real distribution and mt to generate random number instead of rand()
// for better performance and more uniform distribution
SbTrafficMatrix GenerateRandomMatrix(int num_sb) {
  // initial random
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<double> dist(0.0, 10.0);
  // generate matrix
  SbTrafficMatrix matrix = {};
  for (int src_sb = 0; src_sb < num_sb; ++src_sb) {
    for (int dst_sb = 0; dst_sb < num_sb; ++dst_sb) {
      matrix[src_sb * num_sb + dst_sb] = dist(mt);
    }
  }
  return matrix;
}

// Only one element exists in the matrix
SbTrafficMatrix GenerateSparseMatrix(int num_sb) {
  // generate matrix
  SbTrafficMatrix matrix = {};
  for (int src_sb = 0; src_sb < num_sb; ++src_sb) {
    for (int dst_sb = 0; dst_sb < num_sb; ++dst_sb) {
      if ((src_sb == 0) && (dst_sb == 1))
        matrix[src_sb * num_sb + dst_sb] = 10.00;
      else
        matrix[src_sb * num_sb + dst_sb] = 0.00;
    }
  }
  return matrix;
}

// permutation: src_sb + dst_sb == num_sb
SbTrafficMatrix GeneratePermutationMatrix(int num_sb) {
  // generate matrix
  SbTrafficMatrix matrix = {};
  for (int src_sb = 0; src_sb < num_sb; ++src_sb) {
    for (int dst_sb = 0; dst_sb < num_sb; ++dst_sb) {
      if (src_sb + dst_sb == num_sb)
        matrix[src_sb * num_sb + dst_sb] = 10.00;
      else
        matrix[src_sb * num_sb + dst_sb] = 0.00;
    }
  }
  return matrix;
}

// each source SuperBlock sends traffic to the next SuperBlock
SbTrafficMatrix GenerateStrideMatrix(int num_sb) {
  // generate matrix
  SbTrafficMatrix matrix = {};
  for (int src_sb = 0; src_sb < num_sb; ++src_sb) {
    for (int dst_sb = 0; dst_sb < num_sb; ++dst_sb) {
      if ((src_sb + 1) % num_sb == dst_sb)
        matrix[src_sb * num_sb + dst_sb] = 10.00;
      else
        matrix[src_sb * num_sb + dst_sb] = 0.00;
    }
  }
  return matrix;
}

// the interface to generate the traffic matrix
SbTrafficMatrix
Trace::GenerateTrafficMatrix(int num_sb, TrafficPattern traffic_pattern) {
  // input check
  if (num_sb <= 1) {
    LOG(ERROR) << "The number of SuperBlocks cannot be less than 2.";
  }

  // generate matrix according to the traffic pattern
  SbTrafficMatrix matrix = {};
  switch (traffic_pattern) {
    case TrafficPattern::kSparse:
      matrix = GenerateSparseMatrix(num_sb);
      break;
    case TrafficPattern::kPermutation:
      matrix = GeneratePermutationMatrix(num_sb);
      break;
    case TrafficPattern::kStride:
      matrix = GenerateStrideMatrix(num_sb);
      break;
    case TrafficPattern::kSymmetric:
      matrix = GenerateSymmetricMatrix(num_sb);
      break;
    case TrafficPattern::kRandom:
      matrix = GenerateRandomMatrix(num_sb);
      break;
    case TrafficPattern::kUnknown:
      break;
  }
  return matrix;
}

void Trace::ExportTrafficTrace(int num_sb, TrafficPattern traffic_pattern,
                               std::string output) {
  SbTrafficMatrix matrix = GenerateTrafficMatrix(num_sb, traffic_pattern);
  // export the result if the output field is not "none"
  if (output != "none") {
    // test code
    // set the output file
    const char *filename = output.c_str();
    std::fstream fd(filename);
    // write data into proto
    Matrix mx;
    for (auto it=matrix.begin(); it!=matrix.end(); ++it) {
      mx.mutable_traffic_map()->insert({it->first, it->second});
    }
    mx.SerializeToOstream(&fd);
    google::protobuf::ShutdownProtobufLibrary();
    std::cout << "output successfully." << std::endl;
  }
}

SbTrafficMatrix Trace::ImportTrafficTrace(std::string input) {
  SbTrafficMatrix matrix;
  // parse the matrix from the file
  std::fstream fd(input.c_str(), std::ios::in | std::ios::binary);
  Matrix mx;
  mx.ParseFromIstream(&fd);
  for (auto it=mx.traffic_map().cbegin(); it!=mx.traffic_map().cend(); ++it) {
    matrix[it->first] = it->second;
    std::cout << it->first << " " << it->second << std::endl;
  }
  google::protobuf::ShutdownProtobufLibrary();
  return matrix;
}

} // namespace traffic
} // namespace wcmp