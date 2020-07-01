//
// Created by wwt on 6/20/20.
//

#include <random>
#include <glog/logging.h>
#include "Trace.h"

// set the traffic amount to be 10
std::unordered_map<int, double> generate_symmetric_matrix(int num_sb) {
	std::unordered_map<int, double> matrix = {};
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			matrix[src_sb*num_sb+dst_sb] = 10;
		}
	}
	return matrix;
}

// use real distribution and mt to generate random number instead of rand()
// for better performance and more uniform distribution
std::unordered_map<int, double> generate_random_matrix(int num_sb) {
	// initial random
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<double> dist(0.0, 10.0);
	// generate matrix
	std::unordered_map<int, double> matrix = {};
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			matrix[src_sb*num_sb+dst_sb] = dist(mt);
		}
	}
	return matrix;
}

// Only one element exists in the matrix
std::unordered_map<int, double> generate_sparse_matrix(int num_sb) {
	// generate matrix
	std::unordered_map<int, double> matrix = {};
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			if ((src_sb==0) && (dst_sb==1))
				matrix[src_sb*num_sb+dst_sb] = 10.00;
			else
				matrix[src_sb*num_sb+dst_sb] = 0.00;
		}
	}
	return matrix;
}

// permutation: src_sb + dst_sb == num_sb
std::unordered_map<int, double> generate_permutation_matrix(int num_sb) {
	// generate matrix
	std::unordered_map<int, double> matrix = {};
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			if (src_sb+dst_sb==num_sb)
				matrix[src_sb*num_sb+dst_sb] = 10.00;
			else
				matrix[src_sb*num_sb+dst_sb] = 0.00;
		}
	}
	return matrix;
}

// each source SuperBlock sends traffic to the next SuperBlock
std::unordered_map<int, double> generate_stride_matrix(int num_sb) {
	// generate matrix
	std::unordered_map<int, double> matrix = {};
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			if ((src_sb+1)%num_sb == dst_sb)
				matrix[src_sb*num_sb+dst_sb] = 10.00;
			else
				matrix[src_sb*num_sb+dst_sb] = 0.00;
		}
	}
	return matrix;
}

// the interface to generate the traffic matrix
std::unordered_map<int, double> Trace::generate_traffic_matrix(int num_sb, TrafficPattern traffic_pattern, std::string output) {
	// input check
	if (num_sb <= 1) {
		LOG(ERROR) << "The number of SuperBlocks cannot be less than 2.";
	}

	// generate matrix according to the traffic pattern
	std::unordered_map<int, double> matrix = {};
	switch (traffic_pattern) {
		case TrafficPattern::sparse:
			matrix = generate_sparse_matrix(num_sb);
			break;
		case TrafficPattern::permutation:
			matrix = generate_permutation_matrix(num_sb);
			break;
		case TrafficPattern::stride:
			matrix = generate_stride_matrix(num_sb);
			break;
		case TrafficPattern::symmetric:
			matrix = generate_symmetric_matrix(num_sb);
			break;
		case TrafficPattern::random:
			matrix = generate_random_matrix(num_sb);
			break;
		case TrafficPattern::unknown:
			break;
	}
	// export the result if the output field is not "none"
	if (output != "none") {
		// need implementation with protobuf
		// added soon
	}
	return matrix;
}

std::unordered_map<int, double> Trace::ImportTrafficTrace(std::string input) {
	// need implementation with protobuf
	// added soon
}

