//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_INTRADOMAINTOPOLOGY_H
#define WCMP_INTRADOMAINTOPOLOGY_H


#include <unordered_map>
#include "topology/AbstractTopology.h"

namespace wcmp {
namespace topo {
namespace intradomain {

// the number of s3 per MiddleBlock
const int numS3PerMb = 8;
// the number of s2 per MiddleBlock
const int numS2PerMb = 8;
// the number of s1 per MiddleBlock
const int numS1PerMb = 64;
// the bandwidth for links between S1 and S2
const int S1S2Bandwidth = 100;
// the bandwidth for links between S3 and S1
const int S2S3Bandwidth = 200;
// the number of prefix
const int numPrefix = 4;

// the DCN topology only contain the DCN level switches and links
class IntraDomainTopology : public AbstractTopology {

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
  IntraDomainTopology();

  // print the path with the link name
  void PrintPath(const Path &path);
  // print functions
  void PrintSwitch(int sw);
  void PrintLink(int l);
  void PrintAllLinks();

};

} // namespace intradomain
} // namespace topo
} // namespace wcmp

#endif //WCMP_INTRADOMAINTOPOLOGY_H
