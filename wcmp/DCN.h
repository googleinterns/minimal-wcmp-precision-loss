//
// Created by wwt on 6/14/20.
//

#ifndef WCMP_DCN_H
#define WCMP_DCN_H


#include <iostream>
#include <vector>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include "SuperBlock.h"

// Define a DCN network with a number of SuperBlocks,
// each of them has different northbound bandwidth
class DCN {

private:
	std::vector<SuperBlock *> sb_list_;

public:
	// the initial function of DCN class
	DCN();
	void add_superblock(double link_speed);
	SCIP_RETCODE find_best_dcn_routing(double send_speed);

};


#endif //WCMP_DCN_H
