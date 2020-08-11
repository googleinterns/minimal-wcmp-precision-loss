//
// Created by wwt on 8/8/20.
//

#ifndef WCMP_PATHBASEDILPSOLVER_H
#define WCMP_PATHBASEDILPSOLVER_H

#include "AbstractSolver.h"

namespace wcmp {
namespace solver {

class PathBasedILPSolver : public AbstractSolver {

private:

  std::vector<std::vector<std::vector<double>>> scip_result_;
  double res_u_;
  std::vector<std::vector<std::vector<double>>> res_f_;
  std::vector<std::vector<std::vector<double>>> res_x_;
  std::vector<std::vector<std::vector<double>>> res_y_;

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

public:
  PathBasedILPSolver(wcmp::topo::AbstractTopology topology, std::unordered_map<int, double> &trace);
  SCIP_RETCODE FindBestRouting();

};

} // namespace solver
} // namespace wcmp

#endif //WCMP_PATHBASEDILPSOLVER_H
