//
// Created by wwt on 6/20/20.
//

#include "FullTopology.h"

static double test_SBs[5] = {40, 40, 100, 100, 200};
static double sb_pair_demand = 10;
static double traffic_matrix[25] = {40, 40, 40, 40, 40,
                             40, 40, 40, 40, 40,
                             40, 40, 40, 40, 40,
                             40, 40, 40, 40, 40,
                             40, 40, 40, 40, 40};

FullTopology::FullTopology() {
	// initial traffic matrix
	Trace trace;
	traffic_matrix = trace.generate_sparse_matrix(NUM_SB_PER_DCN);

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
	// initialize the dcn path vectors
	dcn_path_list = std::vector<std::vector<std::vector<Path *>>>(NUM_SB_PER_DCN,std::vector<std::vector<Path *>>(NUM_SB_PER_DCN));

	// list and store all the DCN paths
	for (int src_sb=0; src_sb<NUM_SB_PER_DCN; ++src_sb)
		for (int dst_sb=0; dst_sb<NUM_SB_PER_DCN; ++dst_sb)
			if (dst_sb != src_sb) {
				int index = 0;
				// add the direct paths
				std::vector<Link *> direct_paths;
				direct_paths = this->find_links(src_sb, dst_sb);
				for (Link* l : direct_paths) {
					Path* tmp_path = new Path(index, l);
					++index;
					dcn_path_list[src_sb][dst_sb].push_back(tmp_path);
					l->add_path(tmp_path);
				}
				// add the transit paths
				for (int trans_sb = 0; trans_sb < NUM_SB_PER_DCN; ++trans_sb) {
					if ((trans_sb != src_sb) && (trans_sb != dst_sb)) {
						std::vector<Link *> first_hops;
						std::vector<Link *> second_hops;
						first_hops = this->find_links(src_sb, trans_sb);
						second_hops = this->find_links(trans_sb, dst_sb);
						for (Link *first : first_hops) {
							for (Link *second : second_hops) {
								std::vector<Link *> trans_path;
								trans_path.push_back(first);
								trans_path.push_back(second);
								Path *new_path = new Path(index, trans_path);
								++index;
								dcn_path_list[src_sb][dst_sb].push_back(new_path);
								for (Link* l : trans_path)
									l->add_path((new_path));
							}
						}
					}
				}
			}
	std::cout << "Topology initialized" << std::endl;
}

std::vector<Link *> FullTopology::find_links(int src_sb, int dst_sb) {
	std::vector<Link *> result;
	for (Link* link : dcn_link_list) {
		if ((link->source->superblock_id==src_sb) && (link->destination->superblock_id==dst_sb))
			result.push_back(link);
	}
	return result;
}

// WIP: need to change the params and codes
std::vector<Link *> FullTopology::find_intra_links(int src_sb, int dst_sb) {
	std::vector<Link *> result;
	for (Link* link : dcn_link_list) {
		if ((link->source->superblock_id==src_sb) && (link->destination->superblock_id==dst_sb))
			result.push_back(link);
	}
	return result;
}

std::vector<std::vector<Link *>> FullTopology::find_paths(int src_sb, int dst_sb) {
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
std::vector<std::vector<Link *>> FullTopology::find_intra_paths(int src_sb, int dst_sb) {
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

void FullTopology::print_path(std::vector<Link *> path) {
	for (Link* link : path) {
		std::cout << link->name << " => ";
	}
	std::cout<< "END" << std::endl;
}

// find the best routing policy in the DCN level
SCIP_RETCODE FullTopology::find_best_dcn_routing() {
	int num_sb = NUM_SB_PER_DCN; // get the number of SuperBlocks

	SCIP* scip = nullptr;
	SCIP_CALL(SCIPcreate(&scip)); // create the SCIP environment

	SCIP_CALL(SCIPincludeDefaultPlugins(scip)); // include default plugins
	SCIP_CALL(SCIPcreateProbBasic(scip, "MLU_DCN")); // create the SCIP problem
	SCIP_CALL(SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE)); // set object sense to be minimize

	std::cout << "SCIP setup successfully" << std::endl;

	SCIP_VAR* u; // MLU
	SCIP_CALL(SCIPcreateVarBasic(scip,
	                             &u, // variable
	                             "MLU", // name
	                             0.0, // lower bound
	                             1.0, // upper bound
	                             1.0, // objective
	                             SCIP_VARTYPE_CONTINUOUS)); // variable type
	SCIP_CALL(SCIPaddVar(scip, u));  //Adding the variable

	std::vector<std::vector<std::vector<SCIP_VAR*>>> x; // initialize the variables
	x = std::vector<std::vector<std::vector<SCIP_VAR*>>>(NUM_SB_PER_DCN,std::vector<std::vector<SCIP_VAR *>>(NUM_SB_PER_DCN));
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			if (src_sb == dst_sb) continue;
			std::vector<Path *> paths;
			paths = dcn_path_list[src_sb][dst_sb];
			for (int p=0; p<paths.size(); ++p) {
				x[src_sb][dst_sb].emplace_back((SCIP_VAR *) nullptr);
				std::stringstream ss;
				ss << "x_" << src_sb << "_" << dst_sb << "_" << p;
				SCIP_CALL(SCIPcreateVarBasic(scip,
				                             &x[src_sb][dst_sb][p], // variable
				                             ss.str().c_str(), // name
				                             0.0, // lower bound
				                             1.0, // upper bound
				                             0.0, // objective
				                             SCIP_VARTYPE_CONTINUOUS)); // variable type
				SCIP_CALL(SCIPaddVar(scip, x[src_sb][dst_sb][p]));  //Adding the variable
			}
		}
	}

	std::cout << "Variables created" << std::endl;

	std::vector<SCIP_CONS*> equal_cons; // set the constraint Sum(x_{ij})=1
	int cnt = 0;
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0; dst_sb<num_sb; ++dst_sb) {
			if (src_sb == dst_sb) continue; // skip the self loop
			equal_cons.emplace_back((SCIP_CONS *) nullptr); // add constraint
			std::vector<Path *> paths;
			paths = dcn_path_list[src_sb][dst_sb];
			SCIP_Real values[paths.size()];
			for (int k=0; k<paths.size(); ++k) values[k]=1.0;
			std::stringstream ss;
			ss << "cons_equal1_" << src_sb << "_" << dst_sb;
			SCIP_CALL(SCIPcreateConsBasicLinear(scip,
			                                    &equal_cons[cnt], // constraint
			                                    ss.str().c_str(), // name
			                                    paths.size(), // how many variables
			                                    &x[src_sb][dst_sb][0], // array of pointers to various variables
			                                    values, // array of values of the coefficients of corresponding variables
			                                    1, // LHS of the constraint
			                                    1)); // RHS of the constraint
			SCIP_CALL(SCIPaddCons(scip, equal_cons[cnt]));
			++cnt;
		}
	}

	std::cout << "Equal constraints created." << std::endl;

	std::vector<SCIP_CONS*> link_cons; // set the constraint: link utilization < u
	for (int i=0; i<dcn_link_list.size(); ++i) {
		link_cons.emplace_back((SCIP_CONS *) nullptr); // add constraint
		std::vector<SCIP_VAR*> vars;
		std::vector<SCIP_Real> values;

		// iterate all the paths to check whether contain that link
		for (Path* path : dcn_link_list[i]->related_dcn_paths) {
			int src_sb = path->src_sw->superblock_id;
			int dst_sb = path->dst_sw->superblock_id;
			vars.push_back(x[src_sb][dst_sb][path->index]);
			values.push_back(traffic_matrix[src_sb][dst_sb]/double(dcn_link_list[i]->capacity));
		}

		SCIP_VAR* scip_vars[vars.size()+1];
		for (int v=0; v<vars.size(); ++v) scip_vars[v] = vars[v];
		SCIP_Real scip_values[values.size()+1];
		for (int v=0; v<values.size(); ++v) scip_values[v] = -values[v];

		// add u
		scip_vars[vars.size()] = u;
		scip_values[vars.size()] = 1;

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

	std::cout << "Link constraints created" << std::endl;

	// Release the constraints
	for (SCIP_CONS* con : equal_cons) {
		SCIP_CALL(SCIPreleaseCons(scip, &con));
	}
	for (SCIP_CONS* con : link_cons) {
		SCIP_CALL(SCIPreleaseCons(scip, &con));
	}
	std::cout << "Constraints released" << std::endl;

	// Solve the problem
	SCIP_CALL(SCIPsolve(scip));

	// Get the solutions
	SCIP_SOL* sol = nullptr;
	sol = SCIPgetBestSol(scip);

	scip_result = std::vector<std::vector<std::vector<double>>>(NUM_SB_PER_DCN,std::vector<std::vector<double>>(NUM_SB_PER_DCN));
	std::cout << "problem result: " << SCIPgetSolVal(scip, sol, u) << std::endl;
	for (int src_sb=0; src_sb<NUM_SB_PER_DCN; ++src_sb)
		for (int dst_sb=0; dst_sb<NUM_SB_PER_DCN; ++dst_sb) {
			if (src_sb==dst_sb) continue;
			std::cout << src_sb << "->" << dst_sb << ": ";
			for (Path* p : dcn_path_list[src_sb][dst_sb]) {
				std::cout << SCIPgetSolVal(scip, sol, x[src_sb][dst_sb][p->index]) << ", ";
				scip_result[src_sb][dst_sb].push_back(SCIPgetSolVal(scip, sol, x[src_sb][dst_sb][p->index]));
			}
			std::cout << std::endl;
		}


	SCIP_CALL((SCIPwriteOrigProblem(scip, "MLU.lp", nullptr, FALSE)));
	for (int src_sb=0; src_sb<num_sb; ++src_sb) {
		for (int dst_sb=0;dst_sb<num_sb; ++dst_sb) {
			for (SCIP_VAR* v : x[src_sb][dst_sb])
				SCIP_CALL(SCIPreleaseVar(scip, &v));
		}
	}
	SCIP_CALL(SCIPreleaseVar(scip, &u));
	SCIP_CALL(SCIPfree(&scip));
	return SCIP_OKAY;
}

// TODO: WCMP weight can be calculated, but it is ignored here
// TODO: reason given in the comments following
void FullTopology::result_analysis() {
	std::cout << "The path with 0 traffic is not printed. " << std::endl;
	for (int src_sb=0; src_sb<NUM_SB_PER_DCN; ++src_sb)
		for (int dst_sb=0; dst_sb<NUM_SB_PER_DCN; ++dst_sb)
			if (src_sb != dst_sb) {
				// print the traffic amount for each link
				for (Path* p : dcn_path_list[src_sb][dst_sb]) {
					double traffic_amount = traffic_matrix[src_sb][dst_sb]*scip_result[src_sb][dst_sb][p->index];
					if (traffic_amount > 0) {
						std::cout << traffic_amount << " Gbps of demand from u" << src_sb << "->" << dst_sb
						          << "is placed on DCN " << p->links[0]->name << std::endl;
					}
				}
				// print the WCMP group weight for each switch
				// some doubts remaining, I still hold the problem about how to divide the groups, I will
				// make a slides to show my concern on Wednesday's meeting
			}
}
