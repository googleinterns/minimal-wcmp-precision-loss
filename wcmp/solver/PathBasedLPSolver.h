//
// Created by wwt on 8/8/20.
//

#ifndef WCMP_PATHBASEDLPSOLVER_H
#define WCMP_PATHBASEDLPSOLVER_H

#include "AbstractSolver.h"

namespace wcmp {
namespace solver {

class PathBasedLPSolver : public AbstractSolver {

private:
  // data structures to receive the scip result
  std::vector<std::vector<std::vector<double>>> scip_result_;

  // functions to create the Path-based LP model
  // detailed comments in the function definition
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

public:
  // the construction function
  PathBasedLPSolver(wcmp::topo::AbstractTopology topology, std::unordered_map<int, double> &trace);
  // find the best Routing strategy with the input
  SCIP_RETCODE FindBestRouting();
  // analysis the result of the SCIP
  void ResultAnalysis();

};

} // namespace solver
} // namespace wcmp

#endif //WCMP_PATHBASEDLPSOLVER_H
