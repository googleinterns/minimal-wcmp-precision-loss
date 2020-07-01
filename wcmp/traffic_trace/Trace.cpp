//
// Created by wwt on 6/20/20.
//

#include <random>
#include <glog/logging.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Trace.h"
//#include "trace.pb.h"

// set the traffic amount to be 10
std::unordered_map<int, double> GenerateSymmetricMatrix(int num_sb) {
	std::unordered_map<int, double> matrix = {};
	for (int src_sb = 0; src_sb < num_sb; ++src_sb) {
		for (int dst_sb = 0; dst_sb < num_sb; ++dst_sb) {
			matrix[src_sb * num_sb + dst_sb] = 10;
		}
	}
	return matrix;
}

// use real distribution and mt to generate random number instead of rand()
// for better performance and more uniform distribution
std::unordered_map<int, double> GenerateRandomMatrix(int num_sb) {
	// initial random
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<double> dist(0.0, 10.0);
	// generate matrix
	std::unordered_map<int, double> matrix = {};
	for (int src_sb = 0; src_sb < num_sb; ++src_sb) {
		for (int dst_sb = 0; dst_sb < num_sb; ++dst_sb) {
			matrix[src_sb * num_sb + dst_sb] = dist(mt);
		}
	}
	return matrix;
}

// Only one element exists in the matrix
std::unordered_map<int, double> GenerateSparseMatrix(int num_sb) {
	// generate matrix
	std::unordered_map<int, double> matrix = {};
	for (int src_sb = 0; src_sb < num_sb; ++src_sb) {
		for (int dst_sb = 0; dst_sb < num_sb; ++dst_sb) {
			if ((src_sb == 0) && (dst_sb == 1))
				matrix[src_sb * num_sb + dst_sb] = 10.00;
			else
				matrix[src_sb * num_sb + dst_sb] = 0.00;
		}
	}
	return matrix;
}

// permutation: src_sb + dst_sb == num_sb
std::unordered_map<int, double> GeneratePermutationMatrix(int num_sb) {
	// generate matrix
	std::unordered_map<int, double> matrix = {};
	for (int src_sb = 0; src_sb < num_sb; ++src_sb) {
		for (int dst_sb = 0; dst_sb < num_sb; ++dst_sb) {
			if (src_sb + dst_sb == num_sb)
				matrix[src_sb * num_sb + dst_sb] = 10.00;
			else
				matrix[src_sb * num_sb + dst_sb] = 0.00;
		}
	}
	return matrix;
}

// each source SuperBlock sends traffic to the next SuperBlock
std::unordered_map<int, double> GenerateStrideMatrix(int num_sb) {
	// generate matrix
	std::unordered_map<int, double> matrix = {};
	for (int src_sb = 0; src_sb < num_sb; ++src_sb) {
		for (int dst_sb = 0; dst_sb < num_sb; ++dst_sb) {
			if ((src_sb + 1) % num_sb == dst_sb)
				matrix[src_sb * num_sb + dst_sb] = 10.00;
			else
				matrix[src_sb * num_sb + dst_sb] = 0.00;
		}
	}
	return matrix;
}

// the interface to generate the traffic matrix
std::unordered_map<int, double>
Trace::GenerateTrafficMatrix(int num_sb, TrafficPattern traffic_pattern,
                             std::string output) {
	// input check
	if (num_sb <= 1) {
		LOG(ERROR) << "The number of SuperBlocks cannot be less than 2.";
	}

	// generate matrix according to the traffic pattern
	std::unordered_map<int, double> matrix = {};
	switch (traffic_pattern) {
		case TrafficPattern::sparse:
			matrix = GenerateSparseMatrix(num_sb);
			break;
		case TrafficPattern::permutation:
			matrix = GeneratePermutationMatrix(num_sb);
			break;
		case TrafficPattern::stride:
			matrix = GenerateStrideMatrix(num_sb);
			break;
		case TrafficPattern::symmetric:
			matrix = GenerateSymmetricMatrix(num_sb);
			break;
		case TrafficPattern::random:
			matrix = GenerateRandomMatrix(num_sb);
			break;
		case TrafficPattern::unknown:
			break;
	}
	// export the result if the output field is not "none"
	if (output != "none") {
//		// test code
//		// set the output file
//		const char *filename = output.c_str();
//		std::fstream fd(filename,
//		                std::ios::out | std::ios::trunc | std::ios::binary);
//		// write data into proto
//		trace::Matrix mx;
//		auto map = mx.matrix();
//		map[0] = 10.0;
//		mx.SerializeToOstream(&fd);
//		google::protobuf::ShutdownProtobufLibrary();
	}
	return matrix;
}

std::unordered_map<int, double> Trace::ImportTrafficTrace(std::string input) {
	// need implementation with protobuf
	// added soon
}

