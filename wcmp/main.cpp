//
// Created by wwt on 6/14/20.
//

#include <iostream>
#include <glog/logging.h>
#include "solver/ArcBasedLPSolver.h"
#include "solver/ArcBasedILPSolver.h"
#include "solver/PathBasedLPSolver.h"
#include "solver/PathBasedILPSolver.h"
#include "solver/IntraDomainILPSolver.h"
#include "topology/AbstractTopology.h"
#include "topology/dcnonly/DCNOnlyTopology.h"
#include "topology/full/FullTopology.h"
#include "topology/intradomain/IntraDomainTopology.h"

int main() {
  // initial glog
  google::InitGoogleLogging("scip");

  // initial traffic matrix
  wcmp::traffic::Trace trace;
  std::unordered_map<int, double> traffic_matrix = trace.GenerateTrafficMatrix(64,
      wcmp::traffic::TrafficPattern::kRandom);

  wcmp::topo::dcnonly::DCNOnlyTopology network;
  // wcmp::topo::intradomain::IntraDomainTopology network;

  SCIP_RETCODE retcode;
  if (wcmp::arc_based && wcmp::integer_LP) {
    wcmp::solver::ArcBasedLPSolver solver(network, traffic_matrix);
    SCIP_RETCODE retcode = solver.FindBestRouting();
  }
  else if (!wcmp::arc_based && wcmp::integer_LP) {
    wcmp::solver::PathBasedLPSolver solver(network, traffic_matrix);
    SCIP_RETCODE retcode = solver.FindBestRouting();
  }
  else if (wcmp::arc_based && !wcmp::integer_LP) {
    wcmp::solver::ArcBasedILPSolver solver(network, traffic_matrix);
    SCIP_RETCODE retcode = solver.FindBestRouting();
  }
  else if (!wcmp::arc_based && !wcmp::integer_LP) {
    wcmp::solver::PathBasedILPSolver solver(network, traffic_matrix);
    SCIP_RETCODE retcode = solver.FindBestRouting();
  }

  return 0;
}