//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_DCNONLYTOPOLOGY_H
#define WCMP_DCNONLYTOPOLOGY_H


#include <unordered_map>
#include "topology/AbstractTopology.h"

namespace wcmp {
namespace topo {
namespace dcnonly {

// the number of SuperBlock per DCN
const int numSbPerDcn = 3;
// the number of MiddleBlock per SuperBlock
const int numMbPerSb = 1;
// the number of s3 per MiddleBlock
const int numS3PerMb = 8;
// the number of DCN link per SuperBlock
const int numLinkPerSb = 8;

// the number of s3 switches per SuperBlock
const int numS3PerSb = numS3PerMb*numMbPerSb;

class DCNOnlyTopology : public AbstractTopology {

private:
  // data structures used during the construction
  std::vector<std::vector<std::vector<int>>> per_pair_links_;

  // functions to create the topology
  void AddSwitches();
  void AddLinks();
  void AddPaths();

public:
  // constructor function
  // initialize the network topology and record the switches and links
  // generate all the paths for the dcn connection
  // and record path in both dcn_path_list and Link objects
  // We will never find path or find links in the rest of the code
  DCNOnlyTopology();

  // print the path with the link name
  void PrintPath(const Path &path);

};

} // namespace dcnonly
} // namespace topo
} // namespace wcmp

#endif //WCMP_DCNONLYTOPOLOGY_H
