//
// Created by wwt on 6/14/20.
//

#ifndef WCMP_SWITCH_H
#define WCMP_SWITCH_H


#include "definitions.h"

// Represent each switch in the DCN network
// Contain switch name with following format:
// sw_{superblock id}_{middleblock id}_{switch id}
// Use switch_type to represent the level of the switch:
// s1, s2, s3
class Switch {

private:
	int switch_id_;
	int middleblock_id_;
	int superblock_id_;
	SwitchType switch_type_;
	std::string switch_name_;

public:
	// constructor function
	// use hierarchy information to locate the switch
	// the mb_id and sb_id are both local index, not global index
	// the switch name is also defined by its location
	Switch(int sw_id, int mb_id, int sb_id, SwitchType sw_type);
	std::string GetSwitchName();
};

#endif //WCMP_SWITCH_H
