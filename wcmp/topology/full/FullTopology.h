//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_FULLTOPOLOGY_H
#define WCMP_FULLTOPOLOGY_H


#include <topology/DCN.h>

#define NUM_SB_PER_DCN 5
#define NUM_MB_PER_SB 1
#define NUM_S1_PER_MB 64
#define NUM_S2_PER_MB 8
#define NUM_S3_PER_MB 8
#define NUM_LINK_PER_SB 512

#define BW_S2_S3 100
#define BW_S1_S2 100

#define NUM_S3_PER_SB (NUM_S3_PER_MB*NUM_MB_PER_SB)
#define NUM_S2_PER_SB (NUM_S2_PER_MB*NUM_MB_PER_SB)
#define NUM_S1_PER_SB (NUM_S1_PER_MB*NUM_MB_PER_SB)

// The DCN network for the Whole NETWORK topology
// Used for Milestone 2, now as WIP
// For Milestone 1, please refer to DCNOnlyTopology
class FullTopology: public DCN {

public:
	std::vector<std::vector<double>> traffic_matrix;
	std::vector<Switch *> s3_list;
	std::vector<Switch *> s2_list;
	std::vector<Switch *> s1_list;
	std::vector<Link *> dcn_link_list;
	std::vector<Link *> mid_link_list;
	std::vector<Link *> tor_link_list;
	std::vector<std::vector<std::vector<Path *>>> dcn_path_list;
	std::vector<std::vector<std::vector<double>>> scip_result;

	// constructor function
	// initialize the network topology and record the switches and links
	// generate all the paths for the dcn connection
	// and record path in both dcn_path_list and Link objects
	// We will never find path or find links in the rest of the code
	FullTopology();

	// Input: source superblock, destination superblock
	// Return: all the direct dcn links between the src and dst
	std::vector<Link *> find_links(int src_sb, int dst_sb);

	std::vector<std::vector<Link *>> find_paths(int src_sb, int dst_sb);

	std::vector<Link *> find_intra_links(int src_sb, int dst_sb);

	std::vector<std::vector<Link *>> find_intra_paths(int src_sb, int dst_sb);

	// print the path with the link name
	void print_path(std::vector<Link *> path);

	SCIP_RETCODE find_best_dcn_routing();

	// Print the traffic allocation details
	void result_analysis();

};


#endif //WCMP_FULLTOPOLOGY_H
