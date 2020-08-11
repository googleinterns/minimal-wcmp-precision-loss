//
// Created by wwt on 6/21/20.
//

#ifndef WCMP_LINK_H
#define WCMP_LINK_H


#include "definitions.h"
#include "Switch.h"
#include "Path.h"

// The class for link components of the DCN network.
// And for each link, the related DCN paths is recorded for
// efficient access.
class Link {

private:
	Switch* source_;
	Switch* destination_;
	double capacity_;
	std::string name_;
	int id_;

public:
	// constructor function
	// represent one link with direction: pointers to source switch, destination switch
	// link capacity is the min(u_i, u_j)
	// link ID is the index in vector dcn_link_list of this link
	Link(Switch* src, Switch* dst, double cap, int ID);

	// get the link source
	Switch* destination
	// get the link destination

};


#endif //WCMP_LINK_H
