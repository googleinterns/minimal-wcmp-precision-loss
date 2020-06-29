//
// Created by wwt on 6/14/20.
//

#include "Switch.h"

// TODO: the switch name can be changed for clearance
Switch::Switch(int sw_id, int mb_id, int sb_id, SwitchType sw_type) {
	switch_id = sw_id;
	middleblock_id = mb_id;
	superblock_id = sb_id;
	switch_type = sw_type;
	switch_name = "sw_"+itoa(sb_id)+"_"+itoa(mb_id)+"_"+itoa(sw_id);
}

