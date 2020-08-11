//
// Created by wwt on 6/28/20.
//

#include "Path.h"
#include "Link.h"

// instead of record the src_sb and dst_sb
// the source switch and destination switch is recorded
Path::Path(int idx, std::vector<Link *> link_list) {
	for (Link* l : link_list)
		links_.push_back(l);
	src_sw_ = link_list[0]->source;
	dst_sw_ = link_list[link_list.size()-1]->destination;
	index_ = idx;
}

// instead of record the src_sb and dst_sb
// the source switch and destination switch is recorded
Path::Path(int idx, Link * link){
	links.push_back(link);
	src_sw = link->source;
	dst_sw = link->destination;
	index = idx;
}