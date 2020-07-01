//
// Created by wwt on 6/20/20.
//

#include <glog/logging.h>
#include "DCNOnlyTopology.h"

namespace dcnonly {

static double test_SBs[10] = {100, 100, 40, 40, 200, 100, 40, 100, 200, 200};

// add all the switches to the network
void DCNOnlyTopology::AddSwitches() {
	int sw_gid = 0;
	per_sb_switches_ = std::vector<std::vector<int>>(numSbPerDcn);

	// add s3 switches
	for (int i = 0; i < numSbPerDcn; ++i)
		for (int j = 0; j < numMbPerSb; ++j)
			for (int k = 0; k < numS3PerMb; ++k) {
				mainprog::Switch tmp_sw = {i, j, k, mainprog::SwitchType::s3,
				                           sw_gid};
				switches_.push_back(tmp_sw);
				per_sb_switches_[i].push_back(sw_gid);
				++sw_gid;
			}
	// add s2 switches
	for (int i = 0; i < numSbPerDcn; ++i)
		for (int j = 0; j < numMbPerSb; ++j)
			for (int k = 0; k < numS2PerMb; ++k) {
				mainprog::Switch tmp_sw = {i, j, k, mainprog::SwitchType::s2,
				                           sw_gid};
				switches_.push_back(tmp_sw);
				per_sb_switches_[i].push_back(sw_gid);
				++sw_gid;
			}
	// add s1 switches
	for (int i = 0; i < numSbPerDcn; ++i)
		for (int j = 0; j < numMbPerSb; ++j)
			for (int k = 0; k < numS1PerMb; ++k) {
				mainprog::Switch tmp_sw = {i, j, k, mainprog::SwitchType::s1,
				                           sw_gid};
				switches_.push_back(tmp_sw);
				per_sb_switches_[i].push_back(sw_gid);
				++sw_gid;
			}
}

// add all the links
void DCNOnlyTopology::AddLinks() {
	int link_gid = 0;
	per_pair_links_ = std::vector<std::vector<std::vector<int>>>(numSbPerDcn,
	  std::vector<std::vector<int>>(numSbPerDcn));

	// determine source and destination superblocks
	for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb)
		for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb)
			if (dst_sb != src_sb)
				// determine source and destination middleblocks
				for (int src_mb = 0; src_mb < numMbPerSb; ++src_mb) {
					int dst_mb = src_mb; // only one color is allowed
					// determine switch
					for (int i = 0; i < numS3PerMb; ++i) {
						// determine the source and destination s3
						int src = src_sb * numS2PerSb + src_mb * numS3PerMb + i;
						int dst = dst_sb * numS2PerSb + dst_mb * numS3PerMb + i;
						mainprog::Link tmp_link = {switches_[src].gid,
						                           switches_[dst].gid,
						                           std::min(test_SBs[src_sb],
						                                    test_SBs[dst_sb]),
						                           link_gid};
						links_.push_back(tmp_link);
						per_pair_links_[src_sb][dst_sb].push_back(link_gid);
						++link_gid;
					}
				}
}

// add all the paths
void DCNOnlyTopology::AddPaths() {
	int path_gid = 0;
	// initialize the dcn path vectors
	per_pair_paths_ = std::vector<std::vector<std::vector<int>>>(numSbPerDcn,
	  std::vector<std::vector<int>>(numSbPerDcn));

	// list and store all the DCN paths
	for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb)
		for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb)
			if (dst_sb != src_sb) {
				int index = 0;
				// add the direct paths
				std::vector<int> direct_paths;
				direct_paths = this->FindLinks(src_sb, dst_sb);
				for (int link_gid : direct_paths) {
					mainprog::Path tmp_path = {{link_gid},
					                           links_[link_gid].src_sw_gid,
					                           links_[link_gid].dst_sw_gid,
					                           index, path_gid};
					paths_.push_back(tmp_path); // add path to the path list
					per_pair_paths_[src_sb][dst_sb].push_back(
							path_gid); // add path to per pair paths
					per_link_paths_[link_gid].push_back(path_gid);
					++index;
					++path_gid;
				}
				// add the transit paths
				for (int trans_sb = 0; trans_sb < numSbPerDcn; ++trans_sb) {
					if ((trans_sb != src_sb) && (trans_sb != dst_sb)) {
						std::vector<int> first_hops;
						std::vector<int> second_hops;
						first_hops = this->FindLinks(src_sb, trans_sb);
						second_hops = this->FindLinks(trans_sb, dst_sb);
						for (int first_gid : first_hops) {
							for (int second_gid : second_hops) {
								mainprog::Path new_path = {
										{first_gid, second_gid},
										links_[first_gid].src_sw_gid,
										links_[second_gid].dst_sw_gid, index,
										path_gid};
								paths_.push_back(new_path);
								per_pair_paths_[src_sb][dst_sb].push_back(
										path_gid);
								per_link_paths_[first_gid].push_back(path_gid);
								per_link_paths_[second_gid].push_back(path_gid);
								++index;
								++path_gid;
							}
						}
					}
				}
			}
}

// constructor function
// the links between s2 and s3, s1 and s2 are ignored here
// the paths between s1 are also ignored
// TODO: will be added for milestone 2
DCNOnlyTopology::DCNOnlyTopology() {
	// initial traffic matrix
	Trace trace;
	traffic_matrix_ = trace.GenerateTrafficMatrix(numSbPerDcn,
	  TrafficPattern::sparse);

	// add switches
	AddSwitches();
	// add DCN links
	AddLinks();
	// add DCN paths
	AddPaths();

	std::cout << "Topology initialized" << std::endl;
}

// return the links between two SuperBlocks
std::vector<int> DCNOnlyTopology::FindLinks(int src_sb, int dst_sb) {
	std::vector<int> result;
	for (int link_gid : per_pair_links_[src_sb][dst_sb]) {
		result.push_back(link_gid);
	}
	return result;
}

// TODO: can be optimized for clearance
void DCNOnlyTopology::PrintPath(const mainprog::Path &path) {
	for (int g : path.link_gid_list) {
		std::cout << links_[g].gid << " => ";
	}
	std::cout << "END" << std::endl;
}

// create variable for MLU
SCIP_RETCODE DCNOnlyTopology::CreateVariableMlu(SCIP *scip, SCIP_VAR *&u) {
	SCIP_CALL(SCIPcreateVarBasic(scip,
	                             &u, // variable
	                             "MLU", // name
	                             0.0, // lower bound
	                             1.0, // upper bound
	                             1.0, // objective
	                             SCIP_VARTYPE_CONTINUOUS)); // variable type
	SCIP_CALL(SCIPaddVar(scip, u));  //Adding the variable
	return SCIP_OKAY;
}

// create variable for weights
SCIP_RETCODE DCNOnlyTopology::CreateVariableWeight(SCIP *scip,
      std::vector<std::vector<std::vector<SCIP_VAR *>>> &x) {
	x = std::vector<std::vector<std::vector<SCIP_VAR *>>>(numSbPerDcn,
	  std::vector<std::vector<SCIP_VAR *>>(numSbPerDcn));
	for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb) {
		for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
			if (src_sb == dst_sb) continue;
			for (int p = 0; p < per_pair_paths_[src_sb][dst_sb].size(); ++p) {
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
				SCIP_CALL(SCIPaddVar(scip,
				                     x[src_sb][dst_sb][p]));  //Adding the variable
			}
		}
	}
	return SCIP_OKAY;
}

// add constraints for Sum(x_{ij})=1
SCIP_RETCODE DCNOnlyTopology::CreateConstraintsEqualToOne(
		SCIP *scip,
		std::vector<SCIP_CONS *> &equal_cons,
		std::vector<std::vector<std::vector<SCIP_VAR *>>> &x) {
	int cnt = 0;
	for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb) {
		for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
			if (src_sb == dst_sb) continue; // skip the self loop
			equal_cons.emplace_back((SCIP_CONS *) nullptr); // add constraint
			int num_paths = per_pair_paths_[src_sb][dst_sb].size();
			SCIP_Real values[num_paths];
			for (int k = 0; k < num_paths; ++k) values[k] = 1.0;
			std::stringstream ss;
			ss << "cons_equal_" << src_sb << "_" << dst_sb;
			SCIP_CALL(SCIPcreateConsBasicLinear(scip,
			                                    &equal_cons[cnt], // constraint
			                                    ss.str().c_str(), // name
			                                    num_paths, // how many variables
			                                    &x[src_sb][dst_sb][0], // array of pointers to various variables
			                                    values, // array of values of the coefficients of corresponding variables
			                                    1, // LHS of the constraint
			                                    1)); // RHS of the constraint
			SCIP_CALL(SCIPaddCons(scip, equal_cons[cnt]));
			++cnt;
		}
	}
	return SCIP_OKAY;
}

// set the constraint: link utilization < u
SCIP_RETCODE DCNOnlyTopology::CreateConstraintsLinkUtilizationBound(
		SCIP *scip,
		std::vector<SCIP_CONS *> &link_cons,
		SCIP_VAR *&u,
		std::vector<std::vector<std::vector<SCIP_VAR *>>> &x) {
	// iterate all the links
	for (int i = 0; i < links_.size(); ++i) {
		link_cons.emplace_back((SCIP_CONS *) nullptr); // add constraint
		std::vector<SCIP_VAR *> vars;
		std::vector<SCIP_Real> values;

		// iterate all the paths to check whether contain that link
		for (int path_gid : per_link_paths_[i]) {
			int src_sb = switches_[paths_[path_gid].src_sw_gid].superblock_id;
			int dst_sb = switches_[paths_[path_gid].dst_sw_gid].superblock_id;
			vars.push_back(x[src_sb][dst_sb][paths_[path_gid].per_pair_id]);
			values.push_back(traffic_matrix_[src_sb * numSbPerDcn + dst_sb] /
			                 double(links_[i].capacity));
		}

		SCIP_VAR *scip_vars[vars.size() + 1];
		for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
		SCIP_Real scip_values[values.size() + 1];
		for (int v = 0; v < values.size(); ++v) scip_values[v] = -values[v];

		// add u
		scip_vars[vars.size()] = u;
		scip_values[vars.size()] = 1;

		std::stringstream ss;
		ss << "cons_link_" << links_[i].gid;
		SCIP_CALL(SCIPcreateConsBasicLinear(scip,
		                                    &link_cons[i], // constraint
		                                    ss.str().c_str(), // name
		                                    vars.size() +
		                                    1, // how many variables
		                                    scip_vars, // array of pointers to various variables
		                                    scip_values, // array of values of the coefficients of corresponding variables
		                                    0, // LHS of the constraint
		                                    1)); // RHS of the constraint
		SCIP_CALL(SCIPaddCons(scip, link_cons[i]));
	}
	return SCIP_OKAY;
}

// find the best routing policy in the DCN level
// redundant constraints have been removed
SCIP_RETCODE DCNOnlyTopology::FindBestDcnRouting() {
	SCIP *scip = nullptr;
	SCIP_CALL(SCIPcreate(&scip)); // create the SCIP environment

	SCIP_CALL(SCIPincludeDefaultPlugins(scip)); // include default plugins
	SCIP_CALL(SCIPcreateProbBasic(scip, "MLU_DCN")); // create the SCIP problem
	SCIP_CALL(SCIPsetObjsense(scip,
	                          SCIP_OBJSENSE_MINIMIZE)); // set object sense to be minimize

	std::cout << "SCIP setup successfully" << std::endl;

	SCIP_RETCODE ret;

	SCIP_VAR *u; // MLU
	ret = CreateVariableMlu(scip, u);
	if (ret != SCIP_OKAY) LOG(ERROR) << "The variable u is wrong.";
	else std::cout << "Variable u created." << std::endl;

	std::vector<std::vector<std::vector<SCIP_VAR *>>> x; // initialize the variables
	ret = CreateVariableWeight(scip, x);
	if (ret != SCIP_OKAY) LOG(ERROR) << "The variable x is wrong.";
	else std::cout << "Variable x created." << std::endl;

	std::vector<SCIP_CONS *> equal_cons;
	ret = CreateConstraintsEqualToOne(scip, equal_cons, x);
	if (ret != SCIP_OKAY) LOG(ERROR) << "The equal constraints is wrong.";
	else std::cout << "Equal constraints created." << std::endl;

	std::vector<SCIP_CONS *> link_cons;
	ret = CreateConstraintsLinkUtilizationBound(scip, link_cons, u, x);
	if (ret != SCIP_OKAY) LOG(ERROR) << "The link constraints is wrong.";
	else std::cout << "Link constraints created" << std::endl;

	SCIP_CALL((SCIPwriteOrigProblem(scip, "MLU_before.lp", nullptr, FALSE)));

	// Release the constraints
	for (SCIP_CONS *con : equal_cons) {
		SCIP_CALL(SCIPreleaseCons(scip, &con));
	}
	for (SCIP_CONS *con : link_cons) {
		SCIP_CALL(SCIPreleaseCons(scip, &con));
	}
	std::cout << "Constraints released" << std::endl;

	// Solve the problem
	SCIP_CALL(SCIPsolve(scip));
	std::cout << "Solved" << std::endl;

	// Get the solutions
	SCIP_SOL *sol = nullptr;
	sol = SCIPgetBestSol(scip);

	scip_result_ = std::vector<std::vector<std::vector<double>>>(numSbPerDcn,
	  std::vector<std::vector<double>>(numSbPerDcn));
	std::cout << "problem result: " << SCIPgetSolVal(scip, sol, u) << std::endl;
	for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb)
		for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
			if (src_sb == dst_sb) continue;
			std::cout << src_sb << "->" << dst_sb << ": ";
			for (int p : per_pair_paths_[src_sb][dst_sb]) {
				std::cout << SCIPgetSolVal(scip, sol,
				                           x[src_sb][dst_sb][paths_[p].per_pair_id])
				          << ", ";
				scip_result_[src_sb][dst_sb].push_back(SCIPgetSolVal(scip, sol,
				  x[src_sb][dst_sb][paths_[p].per_pair_id]));
			}
			std::cout << std::endl;
		}

	SCIP_CALL((SCIPwriteOrigProblem(scip, "MLU.lp", nullptr, FALSE)));
	for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb) {
		for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
			for (SCIP_VAR *v : x[src_sb][dst_sb])
				SCIP_CALL(SCIPreleaseVar(scip, &v));
		}
	}
	SCIP_CALL(SCIPreleaseVar(scip, &u));
	SCIP_CALL(SCIPfree(&scip));
	return SCIP_OKAY;
}

// TODO: WCMP weight can be calculated, but it is ignored here
// TODO: reason given in the comments following
void DCNOnlyTopology::ResultAnalysis() {
	std::cout << "The path with 0 traffic is not printed. " << std::endl;
	for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb)
		for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb)
			if (src_sb != dst_sb) {
				// print the traffic amount for each link
				for (int p : per_pair_paths_[src_sb][dst_sb]) {
					double traffic_amount =
							traffic_matrix_[src_sb * numSbPerDcn + dst_sb] *
							scip_result_[src_sb][dst_sb][paths_[p].per_pair_id];
					if (traffic_amount > 0) {
						std::cout << traffic_amount << " Gbps of demand from u"
						          << src_sb << " -> u" << dst_sb
						          << " is placed on DCN link "
						          << links_[paths_[p].link_gid_list.front()].gid
						          << std::endl;
					}
				}
				// print the WCMP group weight for each switch
				// some doubts remaining, I still hold the problem about
				// how to divide the groups, I will
				// make a slides to show my concern on Wednesday's meeting
			}
}

} // namespace dcnonly