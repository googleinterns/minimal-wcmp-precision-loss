//
// Created by wwt on 8/8/20.
//

#ifndef WCMP_INTRADOMAINSOLVER_H
#define WCMP_INTRADOMAINSOLVER_H

#include "AbstractSolver.h"

namespace wcmp {
namespace solver {

class IntraDomainSolver : public AbstractSolver {

private:

  std::vector<std::vector<std::vector<double>>> scip_result_;
  // the maximum link utilization
  double res_u_;
  // the weight assignment on a certain link for a certain src-dst
  std::vector<std::vector<std::vector<double>>> res_f_;

  // functions to create the Arc-based ILP model
  // detailed comments in the function definition
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
  // the construction function
  IntraDomainSolver(wcmp::topo::AbstractTopology topology, std::unordered_map<int, double> &trace);
  // find the best Routing strategy with the input
  SCIP_RETCODE FindBestRouting();

};

} // namespace solver
} // namespace wcmp

#endif //WCMP_INTRADOMAINSOLVER_H
