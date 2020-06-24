//
// Created by wwt on 6/20/20.
//

#include "Trace.h"

Trace::Trace() = default;

double* Trace::generate_symmetric_matrix(int num_sb) {
	auto matrix = new double[num_sb*num_sb];
	for (int i=0; i<num_sb*num_sb; ++i) {
		matrix[i] = 40;
	}
	return matrix;
}

double* Trace::generate_random_matrix(int num_sb) {
	auto matrix = new double[num_sb*num_sb];
	// implementation
	return matrix;
}



