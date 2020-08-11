//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_FULLTOPOLOGY_H
#define WCMP_FULLTOPOLOGY_H


#include <unordered_map>
#include "topology/AbstractTopology.h"

namespace wcmp {
namespace topo {
namespace full {

// the number of SuperBlock per DCN
const int numSbPerDcn = 3;
// the number of MiddleBlock per SuperBlock
const int numMbPerSb = 1;
// the number of s3 per MiddleBlock
const int numS3PerMb = 8;
// the number of s2 per MiddleBlock
const int numS2PerMb = 2; // 8
// the number of DCN link per SuperBlock
const int numLinkPerSb = 8;

// the number of s2/s3 switches per SuperBlock/DCN
const int numS2PerSb = numS2PerMb * numMbPerSb;
const int numS2PerDCN = numS2PerSb * numSbPerDcn;
const int numS3PerSb = numS3PerMb * numMbPerSb;
const int numS3PerDCN = numS3PerSb * numSbPerDcn;

// The Full topology represent the s2-s2 DCN network
class FullTopology : public AbstractTopology {

private:
  // the links that connected to one switch
  std::vector<std::vector<int>> per_switch_links_;
  // the links between a SuperBlock pair
  std::vector<std::vector<std::vector<int>>> per_sb_pair_links_;
  // the paths between a SuperBlock pair
  std::vector<std::vector<std::vector<int>>> per_sb_pair_paths_;
  // the paths that uses one link
  std::unordered_map<int, std::vector<int>> per_link_paths_;
  // the links between a s3 switch pair
  std::vector<std::vector<std::vector<int>>> per_s3_pair_links_;

  // functions to create the topology
  void AddSwitches();
  void AddLinks();
  void AddPaths();

  // Input: source superblock, destination superblock
  // Return: all the direct dcn links between the src and dst
  std::vector<int> FindLinks(int src_sb, int dst_sb);
  // find the paths between the source s3 switch and destination s3 switch
  std::vector<Path> FindDCNPaths(int src_sw, int dst_sw);

public:
  // constructor function
  // initialize the network topology and record the switches and links
  // generate all the paths for the dcn connection
  // and record path in both dcn_path_list and Link objects
  // We will never find path or find links in the rest of the code
  FullTopology();

  // print functions
  void PrintSwitch(int sw);
  void PrintLink(int l);
  void PrintAllLinks();
  void PrintPath(const Path &path);

};

} // namespace full
} // namespace topo
} // namespace wcmp
#endif //WCMP_FullTOPOLOGY_H
