//
// Created by wwt on 6/14/20.
//

#ifndef WCMP_SWITCH_H
#define WCMP_SWITCH_H


#include "main.h"

class Switch {

public:
	int switch_id;
	int middleblock_id;
	int superblock_id;
	SwitchType switch_type;
	std::string switch_name;

	Switch(int sw_id, int mb_id, int sb_id, SwitchType sw_type);

};


#endif //WCMP_SWITCH_H
