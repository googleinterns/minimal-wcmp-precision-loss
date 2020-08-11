//
// Created by wwt on 8/8/20.
//

#ifndef WCMP_ARCBASEDILPSOLVER_H
#define WCMP_ARCBASEDILPSOLVER_H

#include "AbstractSolver.h"

namespace wcmp {
namespace solver {

class ArcBasedILPSolver : public AbstractSolver {

private:

  std::vector<std::vector<std::vector<double>>> scip_result_;
  double res_u_;
  std::vector<std::vector<std::vector<double>>> res_f_;
  std::vector<std::vector<std::vector<double>>> res_x_;
  std::vector<std::vector<std::vector<double>>> res_y_;

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
  ArcBasedILPSolver(wcmp::topo::AbstractTopology topology, std::unordered_map<int, double> &trace);
  SCIP_RETCODE FindBestRouting();

};

} // namespace solver
} // namespace wcmp

#endif //WCMP_ARCBASEDILPSOLVER_H
