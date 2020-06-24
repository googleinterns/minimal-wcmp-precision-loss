//
// Created by wwt on 6/21/20.
//

#include "Link.h"

Link::Link(Switch* src, Switch* dst, double cap, int ID) {
	source = src;
	destination = dst;
	capacity = cap;
	id = ID;
	name = "link: "+src->switch_name+"->"+dst->switch_name;
}