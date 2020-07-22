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

	// find the best routing policy
	SCIP_RETCODE retcode = network.FindBestDcnRouting_ILP();
	if (retcode != SCIP_OKAY) {
		LOG(ERROR) << "The SCIP program is wrong.";
	}
//
//	// analysis the result
//	network.ResultAnalysis();

	return 0;
}