//
// Created by wwt on 6/14/20.
//

#include "Switch.h"

// TODO: the switch name can be changed for clearance
Switch::Switch(int sw_id, int mb_id, int sb_id, SwitchType sw_type) {
	switch_id_ = sw_id;
	middleblock_id_ = mb_id;
	superblock_id_ = sb_id;
	switch_type_ = sw_type;
	switch_name_ = "sw_"+std::to_string(sb_id)+"_"+std::to_string(mb_id)+"_"+std::to_string(sw_id);
}

std::string Switch::GetSwitchName() {
	return this->switch_name_;
}
