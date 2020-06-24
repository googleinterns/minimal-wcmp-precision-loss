//
// Created by wwt on 6/21/20.
//

#ifndef WCMP_LINK_H
#define WCMP_LINK_H


#include "main.h"
#include "Switch.h"

class Link {

public:
	Switch *source;
	Switch *destination;
	double capacity;
	std::string name;

	Link(Switch* src, Switch* dst, double cap);

};


#endif //WCMP_LINK_H