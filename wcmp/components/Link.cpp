//
// Created by wwt on 6/21/20.
//

#include "Link.h"

// TODO: the link name is now listed as switch name, can be changed
Link::Link(Switch* src, Switch* dst, double cap, int ID) {
	source = src;
	destination = dst;
	capacity = cap;
	id = ID;
	name = "link: "+src->switch_name+"->"+dst->switch_name;
}

// TODO: paths taxonomy is required for achieving best performance for milestone 2
void Link::add_path(Path *p) {
	related_dcn_paths.push_back(p);
}