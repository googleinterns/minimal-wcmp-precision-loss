//
// Created by wwt on 6/14/20.
//

#include "main.h"
#include "components/DCN.h"
#include "topology/dcnonly/DCNOnlyTopology.h"
#include "topology/full/FullTopology.h"

#include <iostream>

// helper function for transferring double value into string
std::string ntoa(double n) {
	std::stringstream s;
	s << n;
	return s.str();
}

// helper function for transferring integer value into string
std::string itoa(int n) {
	std::stringstream s;
	s << n;
	return s.str();
}

int main(int argc, char **argv) {
	// initialize the DCN network
	auto *network = new DCNOnlyTopology();

	// find the best routing policy
	SCIP_RETCODE retcode;
	retcode = network->find_best_dcn_routing();
	if (retcode != SCIP_OKAY) {
		std::cout << "The SCIP program is wrong. " << std::endl;
	}

	// analysis the result
	network->result_analysis();
	return 0;
}