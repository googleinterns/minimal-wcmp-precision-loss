//
// Created by wwt on 6/21/20.
//

#ifndef WCMP_LINK_H
#define WCMP_LINK_H


#include "main.h"
#include "Switch.h"
#include "Path.h"

// The class for link components of the DCN network.
// And for each link, the related DCN paths is recorded for
// efficient access.
class Link {

public:
	Switch *source;
	Switch *destination;
	double capacity;
	std::string name;
	int id;
	std::vector<Path *> related_dcn_paths;

	// constructor function
	// represent one link with direction: pointers to source switch, destination switch
	// link capacity is the min(u_i, u_j)
	// link ID is the index in vector dcn_link_list of this link
	Link(Switch* src, Switch* dst, double cap, int ID);

	// record the related paths' information
	// namely, the path p contains this link
	void add_path(Path* p);

};


#endif //WCMP_LINK_H
