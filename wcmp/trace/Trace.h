//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_TRACE_H
#define WCMP_TRACE_H


#include <unordered_map>
#include <vector>

namespace wcmp {
namespace traffic {

enum class TrafficPattern {
  kUnknown = 0,
  kSparse = 1,
  kPermutation = 2,
  kStride = 3,
  kSymmetric = 4,
  kRandom = 5,
};

// key (int): the integer id of a pair of superblocks
// value (double): the aggregated egress demand of the corresponding superblock pair in Gbps
using SbTrafficMatrix = std::unordered_map<int, double>;

// Generate the Trace for AbstractTopology network
// with 5 patterns:
// sparse: only one pair of SuperBlock is transmitting
// permutation: each SuperBlock is communicating with only one other SuperBlock
// stride: SuperBlock (N) communicates with SuperBlock (N+1)
// symmetric: every SuperBlock sends equal amount of traffic to each other SuperBlock
// random: the traffic amount between each Pair is random
class Trace {

public:
  // constructor function
  Trace() = default;

  // matrix generation function for sparse method
  SbTrafficMatrix GenerateTrafficMatrix(int num_sb,
    TrafficPattern traffic_pattern);

  // export traffic trace
  void ExportTrafficMatrix(int num_sb, TrafficPattern traffic_pattern,
    std::string output);

  // import traffic trace
  SbTrafficMatrix ImportTrafficTrace(std::string input);

};

} // namespace traffic
} // namespace wcmp
#endif //WCMP_TRACE_H
