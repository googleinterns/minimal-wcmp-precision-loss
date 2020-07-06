//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_DCNONLYTOPOLOGY_H
#define WCMP_DCNONLYTOPOLOGY_H


#include <unordered_map>
#include "topology/DCN.h"

namespace wcmp {
namespace topo {
namespace dcnonly {

const int numSbPerDcn = 3;
const int numMbPerSb = 1;
const int numS1PerMb = 64; // 64
const int numS2PerMb = 8; // 8
const int numS3PerMb = 8; // 8

const int numS2PerSb = numS3PerMb * numMbPerSb;

const double infinity = 1e+20;

// The DCN network with only the top level, namely, DCN level
// The initialization of this class will record the paths information as well
// The ResultAnalysis function is used to print out the information
// The SCIP result is stored in the scip_result
class DCNOnlyTopology : public dcn::DCN {

private:
  std::unordered_map<int, double> traffic_matrix_;
  std::vector<Switch> switches_;
  std::vector<Link> links_;
  std::vector<Path> paths_;

  std::vector<std::vector<int>> per_sb_switches_;
  std::vector<std::vector<std::vector<int>>> per_pair_links_;
  std::vector<std::vector<std::vector<int>>> per_pair_paths_;

  std::unordered_map<int, std::vector<int>> per_link_paths_;

  std::vector<std::vector<std::vector<double>>> scip_result_;

  // functions to create the topology
  void AddSwitches();

  void AddLinks();

  void AddPaths();

  // Input: source superblock, destination superblock
  // Return: all the direct dcn links between the src and dst
  std::vector<int> FindLinks(int src_sb, int dst_sb);

  // functions to create the SCIP model
  SCIP_RETCODE CreateVariableMlu(SCIP *scip, SCIP_VAR *&u);

  SCIP_RETCODE CreateVariableWeight(SCIP *scip,
                                    std::vector<std::vector<std::vector<SCIP_VAR *>>> &x);

  SCIP_RETCODE CreateConstraintsEqualToOne(SCIP *scip,
                                           std::vector<SCIP_CONS *> &equal_cons,
                                           std::vector<std::vector<std::vector<SCIP_VAR *>>> &x);

  SCIP_RETCODE CreateConstraintsLinkUtilizationBound(SCIP *scip,
                                                     std::vector<SCIP_CONS *> &equal_cons,
                                                     SCIP_VAR *&u,
                                                     std::vector<std::vector<std::vector<SCIP_VAR *>>> &x);

public:
  // constructor function
  // initialize the network topology and record the switches and links
  // generate all the paths for the dcn connection
  // and record path in both dcn_path_list and Link objects
  // We will never find path or find links in the rest of the code
  DCNOnlyTopology();

  // print the path with the link name
  void PrintPath(const Path &path);

  // Use SCIP to find the best traffic allocation method,
  // follows the LP model on the document.
  SCIP_RETCODE FindBestDcnRouting();

  // Print the traffic allocation details
  void ResultAnalysis();

};

} // namespace dcnonly
} // namespace topo
} // namespace wcmp
#endif //WCMP_DCNONLYTOPOLOGY_H
