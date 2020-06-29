//
// Created by wwt on 6/28/20.
//

#ifndef WCMP_PATH_H
#define WCMP_PATH_H


#include "../main.h"
//#include "Link.h"
#include "Switch.h"

// clarification for extern class
class Link;

// Represent each DCN path. (path between two s3 switches)
// Maintain an index field to represent the index for the
// corresponding src and dst superblock pair, for the purpose
// of accelerating calculation in SCIP
class Path {

public:
	std::vector<Link *> links;
	Switch* src_sw;
	Switch* dst_sw;
	int index;

	// constructor function with a link vector
	// used for transition path or direct path
	// the idx represents the index of this path in dcn_path_list[src_sb][dst_sb]
	//
	// the idx can be used to accelerate the calculation in SCIP without iterate the path list
	Path(int idx, std::vector<Link *> link_list);

	// constructor function with a pointer to link
	// used for direct path
	// the idx represents the index of this path in dcn_path_list[src_sb][dst_sb]
	//
	// the idx can be used to accelerate the calculation in SCIP without iterate the path list
	Path(int idx, Link * link);

};


#endif //WCMP_PATH_H
