//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_DCNONLYTOPOLOGY_H
#define WCMP_DCNONLYTOPOLOGY_H


#include <components/DCN.h>

#define NUM_SB_PER_DCN 3
#define NUM_MB_PER_SB 2
#define NUM_S1_PER_MB 64 // 64
#define NUM_S2_PER_MB 8 // 8
#define NUM_S3_PER_MB 8 // 8
#define NUM_LINK_PER_SB NUM_SB_PER_DCN

#define NUM_S3_PER_SB (NUM_S3_PER_MB*NUM_MB_PER_SB)

// The DCN network with only the top level, namely, DCN level
// The initialization of this class will record the paths information as well
// The result_analysis function is used to print out the information
// The SCIP result is stored in the scip_result
class DCNOnlyTopology: public DCN {

public:
	std::vector<std::vector<double>> traffic_matrix;
	std::vector<Switch *> s3_list;
	std::vector<Switch *> s2_list;
	std::vector<Switch *> s1_list;
	std::vector<Link *> dcn_link_list;
	std::vector<std::vector<std::vector<Path *>>> dcn_path_list;
	std::vector<std::vector<std::vector<double>>> scip_result;

	// constructor function
	// initialize the network topology and record the switches and links
	// generate all the paths for the dcn connection
	// and record path in both dcn_path_list and Link objects
	// We will never find path or find links in the rest of the code
	DCNOnlyTopology();

	// Input: source superblock, destination superblock
	// Return: all the direct dcn links between the src and dst
	std::vector<Link *> find_links(int src_sb, int dst_sb);

	// print the path with the link name
	static void print_path(const std::vector<Link *> &path);

	// Use SCIP to find the best traffic allocation method,
	// follows the LP model on the document.
	SCIP_RETCODE find_best_dcn_routing();

	// Print the traffic allocation details
	void result_analysis();

};


#endif //WCMP_DCNONLYTOPOLOGY_H
