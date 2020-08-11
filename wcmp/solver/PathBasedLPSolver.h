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

  std::vector<std::vector<std::vector<double>>> scip_result_;
  double res_u_;
  std::vector<std::vector<std::vector<double>>> res_f_;

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

public:
  PathBasedLPSolver(wcmp::topo::AbstractTopology topology, std::unordered_map<int, double> &trace);
  SCIP_RETCODE FindBestRouting();

};

} // namespace solver
} // namespace wcmp

#endif //WCMP_PATHBASEDLPSOLVER_H
