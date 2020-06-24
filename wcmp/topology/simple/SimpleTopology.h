//
// Created by wwt on 6/20/20.
//

#ifndef WCMP_SIMPLETOPOLOGY_H
#define WCMP_SIMPLETOPOLOGY_H


#include <components/DCN.h>

#define NUM_SB_PER_DCN 5
#define NUM_MB_PER_SB 1
#define NUM_S1_PER_MB 8 // 64
#define NUM_S2_PER_MB 1 // 8
#define NUM_S3_PER_MB 1 // 8
#define NUM_LINK_PER_SB 512

#define NUM_S3_PER_SB (NUM_S3_PER_MB*NUM_MB_PER_SB)

#define PATH std::vector<Link *>

class SimpleTopology: public DCN {

public:
	std::vector<Switch *> s3_list;
	std::vector<Switch *> s2_list;
	std::vector<Switch *> s1_list;
	std::vector<Link *> dcn_link_list;

	SimpleTopology();
	std::vector<Link *> find_links(int src_sb, int dst_sb);
	std::vector<std::vector<Link *>> find_paths(int src_sb, int dst_sb);
	void print_path(std::vector<Link *> path);
	SCIP_RETCODE find_best_dcn_routing();

};


#endif //WCMP_SIMPLETOPOLOGY_H