//
// Created by wwt on 6/21/20.
//

#include "Link.h"

// TODO: the link name is now listed as switch name, can be changed
Link::Link(Switch* src, Switch* dst, double cap, int ID) {
	source_ = src;
	destination_ = dst;
	capacity_ = cap;
	id_ = ID;
	name_ = "link: "+src->GetSwitchName()+"->"+dst->GetSwitchName();
}