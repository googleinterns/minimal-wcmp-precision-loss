//
// Created by wwt on 6/14/20.
//

#include <iostream>
#include <glog/logging.h>
#include "topology/DCN.h"
#include "topology/dcnonly/DCNOnlyTopology.h"
#include "topology/full/FullTopology.h"

int main() {
	// initial glog
	google::InitGoogleLogging("scip");

  // initialize the DCN network
  wcmp::topo::full::FullTopology network;
	network.PrintAllLinks();

	network.SetProblemScope(wcmp::problem_scope);

	// find the best routing policy
  SCIP_RETCODE retcode;
	if (wcmp::arc_based && wcmp::integer_LP)
	  retcode = network.FindBestDcnRoutingArcILP();
	else if (!wcmp::arc_based && wcmp::integer_LP)
    retcode = network.FindBestDcnRoutingPathILP();
  else if (wcmp::arc_based && !wcmp::integer_LP)
    retcode = network.FindBestDcnRoutingArcLP();
  else
    retcode = network.FindBestDcnRoutingPathLP();

	if (retcode != SCIP_OKAY) {
		LOG(ERROR) << "The SCIP program is wrong.";
	}

	// analysis the result
  if (wcmp::arc_based && wcmp::integer_LP)
    network.ArcILPResultAnalysis();
  else if (!wcmp::arc_based && wcmp::integer_LP)
    network.PathILPResultAnalysis();
  else if (wcmp::arc_based && !wcmp::integer_LP)
    network.ArcLPResultAnalysis();
  else
    network.PathLPResultAnalysis();


	return 0;
}