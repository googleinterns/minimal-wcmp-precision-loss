//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_TRACE_H
#define WCMP_TRACE_H


#include "../main.h"
#include "../topology/simple/SimpleTopology.h"
#include "../topology/aurora/AuroraTopology.h"

class Trace {

public:
	Trace();
	double* generate_symmetric_matrix(int num_sb);
	double* generate_random_matrix(int num_sb);
};


#endif //WCMP_TRACE_H
