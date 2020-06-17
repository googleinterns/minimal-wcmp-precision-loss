//
// Created by wwt on 6/14/20.
//

#include "main.h"
#include "DCN.h"
#include "SuperBlock.h"
#include <iostream>

//double test_SBs[6] = {40, 40, 40, 100, 100, 200};
double test_SBs[3] = {40, 100, 200};
double send_speed = 30;

int main(int argc, char **argv) {
	// Construct the simple network with above test SuperBlocks
	DCN* simple_network = new DCN();
	for (int i=0; i<sizeof(test_SBs)/ sizeof(test_SBs[0]); ++i) {
		simple_network->add_superblock(test_SBs[i]);
	}
	// find the best routing policy
	SCIP_RETCODE retcode;
	retcode = simple_network->find_best_dcn_routing(send_speed);
	if (retcode != SCIP_OKAY) {
		std::cout << "The SCIP program is wrong. " << std::endl;
	}
}