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

	Link(Switch* src, Switch* dst, double cap, int ID);
	void add_path(Path* p);

};


#endif //WCMP_LINK_H
