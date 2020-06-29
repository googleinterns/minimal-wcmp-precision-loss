//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_TRACE_H
#define WCMP_TRACE_H


#include "../main.h"

// Generate the Trace for DCN network
// with 5 patterns:
// sparse: only one pair of SuperBlock is transmitting
// permutation: each SuperBlock is communicating with only one other SuperBlock
// stride: SuperBlock (N) communicates with SuperBlock (N+1)
// symmetric: every SuperBlock sends equal amount of traffic to each other SuperBlock
// random: the traffic amount between each Pair is random
class Trace {

public:
	// constructor function
	Trace();

	// matrix generation function for sparse method
	std::vector<std::vector<double>> generate_sparse_matrix(int num_sb);

	// matrix generation function for permutation method
	std::vector<std::vector<double>> generate_permutation_matrix(int num_sb);

	// matrix generation function for stride method
	std::vector<std::vector<double>> generate_stride_matrix(int num_sb);

	// matrix generation function for symmetric method
	std::vector<std::vector<double>> generate_symmetric_matrix(int num_sb);

	// matrix generation function for random method
	std::vector<std::vector<double>> generate_random_matrix(int num_sb);
};


#endif //WCMP_TRACE_H
