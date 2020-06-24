//
// Created by wwt on 6/14/20.
//

#include "main.h"
#include "components/DCN.h"
#include "topology/simple/SimpleTopology.h"
#include "topology/aurora/AuroraTopology.h"

#include <iostream>

std::string ntoa(double n) {
	std::stringstream s;
	s << n;
	return s.str();
}

std::string itoa(int n) {
	std::stringstream s;
	s << n;
	return s.str();
}

int main(int argc, char **argv) {
	auto *network = new SimpleTopology();
//	auto *network = new AuroraTopology();

	// test functions
	std::vector<PATH> paths = network->find_paths(0,1);
	for (PATH path : paths) network->print_path(path);

	// find the best routing policy
	SCIP_RETCODE retcode;
	retcode = network->find_best_dcn_routing();
	if (retcode != SCIP_OKAY) {
		std::cout << "The SCIP program is wrong. " << std::endl;
	}
	return 0;
}