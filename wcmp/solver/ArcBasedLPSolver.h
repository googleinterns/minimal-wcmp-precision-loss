//
// Created by wwt on 8/8/20.
//

#ifndef WCMP_ARCBASEDLPSOLVER_H
#define WCMP_ARCBASEDLPSOLVER_H

#include "AbstractSolver.h"

namespace wcmp {
namespace solver {

class ArcBasedLPSolver : public AbstractSolver {

private:

  std::vector<std::vector<std::vector<double>>> scip_result_;
  double res_u_;
  std::vector<std::vector<std::vector<double>>> res_f_;

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

public:
  ArcBasedLPSolver(wcmp::topo::AbstractTopology topology, std::unordered_map<int, double> &trace);
  SCIP_RETCODE FindBestRouting();

};

} // namespace solver
} // namespace wcmp

#endif //WCMP_ARCBASEDLPSOLVER_H
