//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_TRACE_H
#define WCMP_TRACE_H


#include <unordered_map>
#include <vector>

enum class TrafficPattern {
  kUnknown = 0,
  kSparse = 1,
  kPermutation = 2,
  kStride = 3,
  kSymmetric = 4,
  kRandom = 5,
};

typedef std::unordered_map<int, double> SbTrafficMatrix;

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
  SbTrafficMatrix
  GenerateTrafficMatrix(int num_sb, TrafficPattern traffic_pattern,
                        std::string output = "none");

  // import traffic trace
  SbTrafficMatrix ImportTrafficTrace(std::string input);
};


#endif //WCMP_TRACE_H
