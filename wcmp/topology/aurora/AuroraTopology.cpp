//
// Created by wwt on 6/20/20.
//

#include "AuroraTopology.h"

static double test_SBs[5] = {40, 40, 100, 100, 200};
static double sb_pair_demand = 10;
static double traffic_matrix[25] = {40, 40, 40, 40, 40,
                             40, 40, 40, 40, 40,
                             40, 40, 40, 40, 40,
                             40, 40, 40, 40, 40,
                             40, 40, 40, 40, 40};

AuroraTopology::AuroraTopology() {
	// add s3 switches
	for (int i=0; i<NUM_SB_PER_DCN; ++i)
		for (int j=0; j<NUM_MB_PER_SB; ++j)
			for (int k=0; k<NUM_S3_PER_MB; ++k) {
				auto *tmp_sw = new Switch(k, j, i, s3);
				s3_list.push_back(tmp_sw);
			}
	// add s2 switches
	for (int i=0; i<NUM_SB_PER_DCN; ++i)
		for (int j=0; j<NUM_MB_PER_SB; ++j)
			for (int k=0; k<NUM_S2_PER_MB; ++k) {
				auto *tmp_sw = new Switch(k, j, i, s2);
				s2_list.push_back(tmp_sw);
			}
	// add s1 switches
	for (int i=0; i<NUM_SB_PER_DCN; ++i)
		for (int j=0; j<NUM_MB_PER_SB; ++j)
			for (int k=0; k<NUM_S1_PER_MB; ++k) {
				auto *tmp_sw = new Switch(k, j, i, s1);
				s1_list.push_back(tmp_sw);
			}
	// add DCN link
	// determine source and destination superblocks
	int cnt = 0;
	for (int src_sb=0; src_sb<NUM_SB_PER_DCN; ++src_sb)
		for (int dst_sb=0; dst_sb<NUM_SB_PER_DCN; ++dst_sb)
			if (dst_sb != src_sb)
				// determine source and destination middleblocks
				for (int src_mb=0; src_mb<NUM_MB_PER_SB; ++src_mb)
					for (int dst_mb=0; dst_mb<NUM_MB_PER_SB; ++dst_mb)
						// determine switch
						for (int src_sw=0; src_sw<NUM_S3_PER_MB; ++src_sw) {
							for (int i=src_sw; i<4; ++i) {
								// determine the source and destination s3
								int src = src_sb*NUM_S3_PER_SB+src_mb*NUM_S3_PER_MB+src_sw;
								int dst = dst_sb*NUM_S3_PER_SB+dst_mb*NUM_S3_PER_MB+((src_sw+i*2)%NUM_S3_PER_MB);
								auto *tmp_link = new Link(s3_list[src], s3_list[dst], std::min(test_SBs[src_sb], test_SBs[dst_sb]), cnt);
								dcn_link_list.push_back(tmp_link);
								++cnt;
							}
						}
	// add links between s2 and s3
	// determine superblocks
	for (int sb=0; sb<NUM_SB_PER_DCN; ++sb)
		// determine middleblocks
		for (int mb=0; mb<NUM_MB_PER_SB; ++mb)
			// determine switch
			for (int s3_sw=0; s3_sw<NUM_S3_PER_MB; ++s3_sw) {
				for (int s2_sw=0; s2_sw<NUM_S2_PER_MB; ++s2_sw) {
					// build bidirection links
					int s3_id = sb*NUM_S3_PER_SB+mb*NUM_S3_PER_MB+s3_sw;
					int s2_id = sb*NUM_S2_PER_SB+mb*NUM_S2_PER_MB+s2_sw;
					auto *tmp_link = new Link(s3_list[s3_id], s2_list[s2_id], BW_S2_S3, cnt);
					mid_link_list.push_back(tmp_link);
				}
			}
	// add links between s1 and s2
	// determine superblocks
	for (int sb=0; sb<NUM_SB_PER_DCN; ++sb)
		// determine middleblocks
		for (int mb=0; mb<NUM_MB_PER_SB; ++mb)
			// determine switch
			for (int s2_sw=0; s2_sw<NUM_S2_PER_MB; ++s2_sw) {
				for (int s1_sw=0; s1_sw<NUM_S3_PER_MB; ++s1_sw) {
					// build bidirection links
					int s2_id = sb*NUM_S3_PER_SB+mb*NUM_S3_PER_MB+s2_sw;
					int s1_id = sb*NUM_S1_PER_SB+mb*NUM_S1_PER_MB+s1_sw;
					auto *tmp_link = new Link(s2_list[s2_id], s1_list[s1_id], BW_S2_S3, cnt);
					tor_link_list.push_back(tmp_link);
				}
			}
}

std::vector<Link *> AuroraTopology::find_links(int src_sb, int dst_sb) {
	std::vector<Link *> result;
	for (Link* link : dcn_link_list) {
		if ((link->source->superblock_id==src_sb) && (link->destination->superblock_id==dst_sb))
			result.push_back(link);
	}
	return result;
}

// WIP: need to change the params and codes
std::vector<Link *> AuroraTopology::find_intra_links(int src_sb, int dst_sb) {
	std::vector<Link *> result;
	for (Link* link : dcn_link_list) {
		if ((link->source->superblock_id==src_sb) && (link->destination->superblock_id==dst_sb))
			result.push_back(link);
	}
	return result;
}

std::vector<std::vector<Link *>> AuroraTopology::find_paths(int src_sb, int dst_sb) {
	std::vector<std::vector<Link *>> result;
	if (src_sb == dst_sb) return result;
	// add the direct paths
	std::vector<Link *> direct_path;
	direct_path = find_links(src_sb, dst_sb);
	for (Link* link : direct_path) {
		std::vector<Link *> tmp_path;
		tmp_path.push_back(link);
		result.push_back(tmp_path);
	}
	// add the transit paths
	for (int trans_sb=0; trans_sb<NUM_SB_PER_DCN; ++trans_sb) {
		if ((trans_sb!=src_sb) && (trans_sb!=dst_sb)) {
			std::vector<Link *> first_hops;
			std::vector<Link *> second_hops;
			first_hops = find_links(src_sb, trans_sb);
			second_hops = find_links(trans_sb, dst_sb);
			for (Link* first : first_hops) {
				for (Link* second : second_hops) {
					std::vector<Link *> tmp_path;
					tmp_path.push_back(first);
					tmp_path.push_back(second);
					result.push_back(tmp_path);
				}
			}
		}
	}
}

// WIP: need to change the params and codes
std::vector<std::vector<Link *>> AuroraTopology::find_intra_paths(int src_sb, int dst_sb) {
	std::vector<std::vector<Link *>> result;
	if (src_sb == dst_sb) return result;
	// add the direct paths
	std::vector<Link *> direct_path;
	direct_path = find_links(src_sb, dst_sb);
	for (Link* link : direct_path) {
		std::vector<Link *> tmp_path;
		tmp_path.push_back(link);
		result.push_back(tmp_path);
	}
	// add the transit paths
	for (int trans_sb=0; trans_sb<NUM_SB_PER_DCN; ++trans_sb) {
		if ((trans_sb!=src_sb) && (trans_sb!=dst_sb)) {
			std::vector<Link *> first_hops;
			std::vector<Link *> second_hops;
			first_hops = find_links(src_sb, trans_sb);
			second_hops = find_links(trans_sb, dst_sb);
			for (Link* first : first_hops) {
				for (Link* second : second_hops) {
					std::vector<Link *> tmp_path;
					tmp_path.push_back(first);
					tmp_path.push_back(second);
					result.push_back(tmp_path);
				}
			}
		}
	}
}

void AuroraTopology::print_path(std::vector<Link *> path) {
	for (Link* link : path) {
		std::cout << link->name << " => ";
	}
	std::cout<< "END" << std::endl;
}

// find the best routing policy in the DCN level
SCIP_RETCODE AuroraTopology::find_best_dcn_routing() {
	std::cout << "begin" << std::endl;
	int num_sb = NUM_SB_PER_DCN; // get the number of SuperBlocks

	SCIP* scip = nullptr;
	SCIP_CALL(SCIPcreate(&scip)); // create the SCIP environment

	SCIP_CALL(SCIPincludeDefaultPlugins(scip)); // include default plugins
	SCIP_CALL(SCIPcreateProbBasic(scip, "MLU")); // create the SCIP problem
	SCIP_CALL(SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE)); // set object sense to be minimize

	std::cout << "create" << std::endl;

	SCIP_VAR* u; // MLU
	SCIP_CALL(SCIPcreateVarBasic(scip,
	                             &u, // variable
	                             "MLU", // name
	                             0.0, // lower bound
	                             1.0, // upper bound
	                             1.0, // objective
	                             SCIP_VARTYPE_CONTINUOUS)); // variable type
	SCIP_CALL(SCIPaddVar(scip, u));  //Adding the variable

	std::vector<std::vector<SCIP_VAR*>> x; // initialize the variables
	for (int i=0; i<num_sb; ++i) {
		for (int j=0; j<num_sb; ++j) {
			x.emplace_back(std::vector<SCIP_VAR*>());
			std::vector<std::vector<Link *>> paths;
			paths = find_paths(i, j);
			for (int p=0; p<paths.size(); ++p) {
				x[i*num_sb+j].emplace_back((SCIP_VAR *) nullptr);
				std::stringstream ss;
				ss << "x_" << i << "_" << j << "_" << p;
				SCIP_CALL(SCIPcreateVarBasic(scip,
				                             &x[i*num_sb+j][p], // variable
				                             ss.str().c_str(), // name
				                             0.0, // lower bound
				                             1.0, // upper bound
				                             0.0, // objective
				                             SCIP_VARTYPE_CONTINUOUS)); // variable type
				SCIP_CALL(SCIPaddVar(scip, x[i*num_sb+j][p]));  //Adding the variable
			}
		}
	}

	std::cout << "create variables" << std::endl;

	std::vector<std::vector<SCIP_CONS*>> cons; // set the constraint 0<x_{ijp}<1
	for (int i=0; i<num_sb; ++i) {
		for (int j=0; j<num_sb; ++j) {
			cons.emplace_back(std::vector<SCIP_CONS*>());
			std::vector<std::vector<Link *>> paths;
			paths = find_paths(i, j);
			for (int p=0; p<paths.size(); ++p) {
				cons[i*num_sb+j].emplace_back((SCIP_CONS *) nullptr); // add constraint

				SCIP_Real values[1]={1.0};
				std::stringstream ss;
				ss << "bound_cons_" << i << "_" << j << "_" << p;
				SCIP_CALL(SCIPcreateConsBasicLinear(scip,
				                                    &cons[i*num_sb+j][p], // constraint
				                                    ss.str().c_str(), // name
				                                    1, // how many variables
				                                    &x[i*num_sb+j][p], // array of pointers to various variables
				                                    values, // array of values of the coefficients of corresponding variables
				                                    0, // LHS of the constraint
				                                    1)); // RHS of the constraint
				SCIP_CALL(SCIPaddCons(scip, cons[i*num_sb+j][p]));
			}
		}
	}

	std::cout << "create bound cons" << std::endl;

	std::vector<SCIP_CONS*> equal_cons; // set the constraint Sum(x_{ij})=1
	for (int i=0; i<num_sb; ++i) {
		for (int j=0; j<num_sb; ++j) {
			equal_cons.emplace_back((SCIP_CONS *) nullptr); // add constraint
			std::vector<std::vector<Link *>> paths;
			paths = find_paths(i, j);

			SCIP_Real values[paths.size()];
			for (int k=0; k<paths.size(); ++k) values[k]=1;
			std::stringstream ss;
			ss << "cons_" << i;
			SCIP_CALL(SCIPcreateConsBasicLinear(scip,
			                                    &equal_cons[i*num_sb+j], // constraint
			                                    ss.str().c_str(), // name
			                                    paths.size(), // how many variables
			                                    &x[i*num_sb+j][0], // array of pointers to various variables
			                                    values, // array of values of the coefficients of corresponding variables
			                                    1, // LHS of the constraint
			                                    1)); // RHS of the constraint
			SCIP_CALL(SCIPaddCons(scip, equal_cons[i*num_sb+j]));
		}
	}

	std::cout << "create equal cons" << std::endl;

	std::vector<SCIP_CONS*> link_cons; // set the constraint: link utilization < u
	for (int i=0; i<dcn_link_list.size(); ++i) {
		link_cons.emplace_back((SCIP_CONS *) nullptr); // add constraint
		std::vector<SCIP_VAR*> vars;
		std::vector<SCIP_Real> values;

		for (int src_sb=0; src_sb<num_sb; ++src_sb) {
			for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
				// iterate all the paths
				std::vector<std::vector<Link *>> paths;
				paths = find_paths(src_sb, dst_sb);
				for (int p=0; p<paths.size(); ++p) {
					for (Link* link : paths[p]) {
						// add variables
						vars.push_back(x[src_sb*num_sb+dst_sb][p]);
						// add coefficients
						values.push_back(traffic_matrix[src_sb*num_sb+dst_sb]/dcn_link_list[i]->capacity);
					}
				}
			}
		}

		SCIP_VAR* scip_vars[vars.size()+1];
		for (int v=0; v<vars.size(); ++v) scip_vars[v] = vars[v];
		SCIP_Real scip_values[values.size()+1];
		for (int v=0; v<values.size(); ++v) scip_values[v] = -values[v];
		// add u
		scip_vars[-1] = u;
		scip_values[-1] = 1;

		std::stringstream ss;
		ss << "cons_" << dcn_link_list[i]->name;
		SCIP_CALL(SCIPcreateConsBasicLinear(scip,
		                                    &link_cons[i], // constraint
		                                    ss.str().c_str(), // name
		                                    vars.size()+1, // how many variables
		                                    scip_vars, // array of pointers to various variables
		                                    scip_values, // array of values of the coefficients of corresponding variables
		                                    0, // LHS of the constraint
		                                    1)); // RHS of the constraint
		SCIP_CALL(SCIPaddCons(scip, link_cons[i]));
	}

	std::cout << "create link cons" << std::endl;

	// Release the constraints
	for (int i=0; i<num_sb; ++i) {
		for (int j=0;j<num_sb; ++j) {
			std::cout << i*num_sb+j << std::endl;
			SCIP_CALL(SCIPreleaseCons(scip, &equal_cons[i*num_sb+j]));
			for (SCIP_CONS* con : cons[i*num_sb+j]) {
				SCIP_CALL(SCIPreleaseCons(scip, &con));
			}
		}
	}
	std::cout << "release constraints 1" << std::endl;
	for (SCIP_CONS* con : link_cons) {
		SCIP_CALL(SCIPreleaseCons(scip, &con));
	}

	std::cout << "release constraints 2" << std::endl;

	// Solve the problem
	SCIP_CALL(SCIPsolve(scip));

	std::cout << "solve the problem" << std::endl;

	// Get the solutions
	SCIP_SOL* sol = nullptr;
	sol = SCIPgetBestSol(scip);

	std::cout << "problem result" << std::endl;

	SCIP_CALL((SCIPwriteOrigProblem(scip, "MLU.lp", nullptr, FALSE)));
	for (int i=0; i<num_sb; ++i) {
		for (int j=0;j<num_sb; ++j) {
			for (SCIP_VAR* v : x[i*num_sb+j])
				SCIP_CALL(SCIPreleaseVar(scip, &v));
		}
	}
	SCIP_CALL(SCIPreleaseVar(scip, &u));
	SCIP_CALL(SCIPfree(&scip));
	return SCIP_OKAY;
}
