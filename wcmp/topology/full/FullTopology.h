//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_FULLTOPOLOGY_H
#define WCMP_FULLTOPOLOGY_H


#include <unordered_map>
#include "topology/DCN.h"

namespace wcmp {
namespace topo {
namespace full {

const int numSbPerDcn = 3;
const int numMbPerSb = 1;
const int numS1PerMb = 2; // 64
const int numS2PerMb = 2; // 8
const int numS3PerMb = 2; // 8

const int numLinkPerSb = 2; //512;

const int numS3PerSb = numS3PerMb * numMbPerSb;
const int numS3PerDCN = numS3PerSb * numSbPerDcn;

const int numSwPerMb = numS1PerMb+numS2PerMb+numS3PerMb;
const int numSwPerSb = numSwPerMb*numMbPerSb;
const int numSwPerDCN = numSwPerSb*numSbPerDcn;

const double infinity = 1e+20;

// The DCN network with only the top level, namely, DCN level
// The initialization of this class will record the paths information as well
// The PathLPResultAnalysis function is used to print out the information
// The SCIP result is stored in the scip_result
class FullTopology : public dcn::DCN {

private:
  std::unordered_map<int, double> traffic_matrix_;

  std::vector<Switch> switches_;
  std::vector<Link> links_;
  std::vector<Path> paths_;

  std::vector<std::vector<std::vector<int>>> per_sb_switches_; // 0: S3, 1: S2, 2: S1, 3: Virt_S3, 4: Virt_S2
  std::vector<int> s3_virtual_switches_;
  std::vector<int> s2_virtual_switches_;

  std::vector<std::vector<int>> per_switch_links_;
  std::unordered_map<int, std::vector<int>> per_sw_pair_links_;
  std::vector<std::vector<std::vector<int>>> per_sb_pair_links_;

  std::vector<std::vector<std::vector<int>>> per_sb_pair_paths_;
  std::unordered_map<int, std::vector<int>> per_link_paths_;

  std::vector<std::vector<std::vector<double>>> scip_result_;
  double res_u_;
  std::vector<std::vector<std::vector<double>>> res_f_;
  std::vector<std::vector<std::vector<double>>> res_x_;
  std::vector<std::vector<std::vector<double>>> res_y_;

  std::vector<std::vector<std::vector<int>>> per_s3_pair_links_;

  // problem scope specific data structure
  std::vector<int> src_sw_group;
  std::vector<int> dst_sw_group;
  std::vector<int> int_sw_group;
  std::vector<int> sw_group;
  std::vector<int> link_group;

  // functions to create the topology
  void AddSwitches();
  void AddLinks();
  void AddPaths();

  // Input: source superblock, destination superblock
  // Return: all the direct dcn links between the src and dst
  std::vector<int> FindLinks(int src_sb, int dst_sb);

  // functions to create the Path-based LP model
  SCIP_RETCODE PathLPCreateVariableGoal(SCIP *scip, SCIP_VAR *&u);
  SCIP_RETCODE PathLPCreateVariableWeight(SCIP *scip,
                                          std::vector<std::vector<std::vector<SCIP_VAR *>>> &x);
  SCIP_RETCODE PathLPCreateConstraintsEqualToOne(SCIP *scip,
                                                 std::vector<SCIP_CONS *> &equal_cons,
                                                 std::vector<std::vector<std::vector<SCIP_VAR *>>> &x);
  SCIP_RETCODE PathLPCreateConstraintsLinkUtilizationBound(SCIP *scip,
                                                           std::vector<SCIP_CONS *> &equal_cons,
                                                           SCIP_VAR *&u,
                                                           std::vector<std::vector<std::vector<SCIP_VAR *>>> &x);

  // functions to create the Arc-based LP model
  SCIP_RETCODE ArcLPCreateVariableGoal(SCIP *scip, SCIP_VAR *&u);
  SCIP_RETCODE ArcLPCreateVariableF(SCIP *scip,
                               std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);
  SCIP_RETCODE ArcLPCreateConstraints1(SCIP *scip,
                                  std::vector<SCIP_CONS *> &cons_1,
                                  std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);
  SCIP_RETCODE ArcLPCreateConstraints2(SCIP *scip,
                                  std::vector<SCIP_CONS *> &cons_2,
                                  std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);
  SCIP_RETCODE ArcLPCreateConstraints9(SCIP *scip,
                                  std::vector<SCIP_CONS *> &cons_9,
                                  SCIP_VAR *&u,
                                  std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);

  // functions to create the Path-based ILP model
  SCIP_RETCODE PathILPCreateVariableGoal(SCIP *scip, SCIP_VAR *&u);
  SCIP_RETCODE PathILPCreateVariableF(SCIP *scip,
                                     std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);
  SCIP_RETCODE PathILPCreateVariableX(SCIP *scip,
                                     std::vector<std::vector<std::vector<SCIP_VAR *>>> &x);
  SCIP_RETCODE PathILPCreateVariableY(SCIP *scip,
                                     std::vector<std::vector<std::vector<SCIP_VAR *>>> &y);
  SCIP_RETCODE PathILPCreateVariableB(SCIP *scip,
                                     std::vector<std::vector<SCIP_VAR *>> &b);
  SCIP_RETCODE PathILPCreateConstraints1(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_1,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);
  SCIP_RETCODE PathILPCreateConstraints2(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_2,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);
  SCIP_RETCODE PathILPCreateConstraints3(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_3,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &f,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &x);
  SCIP_RETCODE PathILPCreateConstraints4(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_4,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &x,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &y);
  SCIP_RETCODE PathILPCreateConstraints5(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_5,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &y,
                                        std::vector<std::vector<SCIP_VAR *>> &b);
  SCIP_RETCODE PathILPCreateConstraints5_modified(SCIP *scip,
                                                 std::vector<SCIP_CONS *> &cons_5,
                                                 std::vector<std::vector<std::vector<SCIP_VAR *>>> &y,
                                                 std::vector<std::vector<SCIP_VAR *>> &b);
  SCIP_RETCODE PathILPCreateConstraints6(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_6,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &f,
                                        std::vector<std::vector<SCIP_VAR *>> &b);
  SCIP_RETCODE PathILPCreateConstraints9(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_9,
                                        SCIP_VAR *&u,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);

  // functions to create the Arc-based ILP model
  SCIP_RETCODE ArcILPCreateVariableGoal(SCIP *scip, SCIP_VAR *&u);
  SCIP_RETCODE ArcILPCreateVariableF(SCIP *scip,
                                     std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);
  SCIP_RETCODE ArcILPCreateVariableX(SCIP *scip,
                                     std::vector<std::vector<std::vector<SCIP_VAR *>>> &x);
  SCIP_RETCODE ArcILPCreateVariableY(SCIP *scip,
                                     std::vector<std::vector<std::vector<SCIP_VAR *>>> &y);
  SCIP_RETCODE ArcILPCreateVariableB(SCIP *scip,
                                     std::vector<std::vector<SCIP_VAR *>> &b);
  SCIP_RETCODE ArcILPCreateConstraints1(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_1,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);
  SCIP_RETCODE ArcILPCreateConstraints2(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_2,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);
  SCIP_RETCODE ArcILPCreateConstraints3(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_3,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &f,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &x);
  SCIP_RETCODE ArcILPCreateConstraints4(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_4,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &x,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &y);
  SCIP_RETCODE ArcILPCreateConstraints5(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_5,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &y,
                                        std::vector<std::vector<SCIP_VAR *>> &b);
  SCIP_RETCODE ArcILPCreateConstraints6(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_6,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &f,
                                        std::vector<std::vector<SCIP_VAR *>> &b);
  SCIP_RETCODE ArcILPCreateConstraints9(SCIP *scip,
                                        std::vector<SCIP_CONS *> &cons_9,
                                        SCIP_VAR *&u,
                                        std::vector<std::vector<std::vector<SCIP_VAR *>>> &f);

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

  // set the problem scope
  void SetProblemScope(int scope);

  // Use SCIP to find the best traffic allocation method,
  // follows the LP model on the document.
  SCIP_RETCODE FindBestDcnRoutingPathLP();
  SCIP_RETCODE FindBestDcnRoutingArcLP();
  SCIP_RETCODE FindBestDcnRoutingPathILP();
  SCIP_RETCODE FindBestDcnRoutingArcILP();

  // Print the traffic allocation details
  void PathLPResultAnalysis();
  void ArcLPResultAnalysis();
  void PathILPResultAnalysis();
  void ArcILPResultAnalysis();

  // Transfer the original group weight to integer group weight
  std::vector<int> OriginMethod(std::vector<int> weights);
  std::vector<int> NewMethod(std::vector<int> weights);
};

} // namespace full
} // namespace topo
} // namespace wcmp
#endif //WCMP_FullTOPOLOGY_H
