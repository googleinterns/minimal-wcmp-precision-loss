//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_TRACE_H
#define WCMP_TRACE_H


#include <unordered_map>
#include <vector>

enum class TrafficPattern {
  unknown = 0,
  sparse = 1,
  permutation = 2,
  stride = 3,
  symmetric = 4,
  random = 5,
};

// Generate the Trace for DCN network
// with 5 patterns:
// sparse: only one pair of SuperBlock is transmitting
// permutation: each SuperBlock is communicating with only one other SuperBlock
// stride: SuperBlock (N) communicates with SuperBlock (N+1)
// symmetric: every SuperBlock sends equal amount of traffic to each other SuperBlock
// random: the traffic amount between each Pair is random
class Trace {

public:
  // constructor function
  Trace() = default;;

  // matrix generation function for sparse method
  std::unordered_map<int, double>
  GenerateTrafficMatrix(int num_sb, TrafficPattern traffic_pattern,
                        std::string output = "none");

  // import traffic trace
  std::unordered_map<int, double> ImportTrafficTrace(std::string input);
};


#endif //WCMP_TRACE_H
