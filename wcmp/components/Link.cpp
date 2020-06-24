//
// Created by wwt on 6/21/20.
//

#include "Link.h"

Link::Link(Switch* src, Switch* dst, double cap) {
	source = src;
	destination = dst;
	capacity = cap;
	name = "link: "+src->switch_name+"->"+dst->switch_name;
}