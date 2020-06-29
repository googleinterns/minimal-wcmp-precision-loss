//
// Created by wwt on 6/28/20.
//

#include "Path.h"
#include "Link.h"

Path::Path(int idx, std::vector<Link *> link_list) {
	for (Link* l : link_list)
		links.push_back(l);
	src_sw = link_list[0]->source;
	dst_sw = link_list[link_list.size()-1]->destination;
	index = idx;
}

Path::Path(int idx, Link * link){
	links.push_back(link);
	src_sw = link->source;
	dst_sw = link->destination;
	index = idx;
}