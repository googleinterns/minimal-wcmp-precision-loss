//
// Created by wwt on 6/20/20.
//

#include "Trace.h"
#include <random>

Trace::Trace() = default;

// set the traffic amount to be 10
std::vector<std::vector<double>> Trace::generate_symmetric_matrix(int num_sb) {
	std::vector<std::vector<double>> matrix = std::vector<std::vector<double>>(num_sb, std::vector<double>(num_sb));
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			matrix[src_sb][dst_sb] = 10;
		}
	}
	return matrix;
}

// use real distribution and mt to generate random number instead of rand()
// for better performance and more uniform distribution
std::vector<std::vector<double>> Trace::generate_random_matrix(int num_sb) {
	// initial random
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<double> dist(0.0, 10.0);
	// generate matrix
	std::vector<std::vector<double>> matrix = std::vector<std::vector<double>>(num_sb, std::vector<double>(num_sb));
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			matrix[src_sb][dst_sb] = dist(mt);
		}
	}
	return matrix;
}

// Only one element exists in the matrix
std::vector<std::vector<double>> Trace::generate_sparse_matrix(int num_sb) {
	// generate matrix
	std::vector<std::vector<double>> matrix = std::vector<std::vector<double>>(num_sb, std::vector<double>(num_sb));
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			if ((src_sb==0) && (dst_sb==1))
				matrix[src_sb][dst_sb] = 10.00;
			else
				matrix[src_sb][dst_sb] = 0.00;
		}
	}
	return matrix;
}

// permutation: src_sb + dst_sb == num_sb
std::vector<std::vector<double>> Trace::generate_permutation_matrix(int num_sb) {
	// generate matrix
	std::vector<std::vector<double>> matrix = std::vector<std::vector<double>>(num_sb, std::vector<double>(num_sb));
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			if (src_sb+dst_sb==num_sb)
				matrix[src_sb][dst_sb] = 10.00;
			else
				matrix[src_sb][dst_sb] = 0.00;
		}
	}
	return matrix;
}

// each source SuperBlock sends traffic to the next SuperBlock
std::vector<std::vector<double>> Trace::generate_stride_matrix(int num_sb) {
	// generate matrix
	std::vector<std::vector<double>> matrix = std::vector<std::vector<double>>(num_sb, std::vector<double>(num_sb));
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			if ((src_sb+1)%num_sb == dst_sb)
				matrix[src_sb][dst_sb] = 10.00;
			else
				matrix[src_sb][dst_sb] = 0.00;
		}
	}
	return matrix;
}



