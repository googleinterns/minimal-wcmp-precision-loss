//
// Created by wwt on 6/20/20.
//


#include <chrono>
#include <glog/logging.h>
#include "FullTopology.h"

namespace wcmp {
namespace topo {
namespace full {

// create variable for MLU
SCIP_RETCODE FullTopology::CreateVariableMlu(SCIP *scip, SCIP_VAR *&u) {
  SCIP_CALL(SCIPcreateVarBasic(scip,
                               &u, // variable
                               "MLU", // name
                               0.0, // lower bound
                               infinity, // upper bound
                               1.0, // objective
                               SCIP_VARTYPE_CONTINUOUS)); // variable type
  SCIP_CALL(SCIPaddVar(scip, u));  //Adding the variable
  return SCIP_OKAY;
}

// create variable for weights
SCIP_RETCODE FullTopology::CreateVariableWeight(SCIP *scip,
                                                std::vector<std::vector<std::vector<SCIP_VAR *>>> &x) {
  x = std::vector<std::vector<std::vector<SCIP_VAR *>>>(numSbPerDcn,
                                                        std::vector<std::vector<SCIP_VAR *>>(
                                                          numSbPerDcn));
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
SCIP_RETCODE FullTopology::CreateConstraintsEqualToOne(
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
SCIP_RETCODE FullTopology::CreateConstraintsLinkUtilizationBound(
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
                                        infinity)); // RHS of the constraint
    SCIP_CALL(SCIPaddCons(scip, link_cons[i]));
  }
  return SCIP_OKAY;
}

// find the best routing policy in the DCN level
SCIP_RETCODE FullTopology::FindBestDcnRouting() {
  SCIP *scip = nullptr;
  SCIP_CALL(SCIPcreate(&scip)); // create the SCIP environment

  SCIP_CALL(SCIPincludeDefaultPlugins(scip)); // include default plugins
  SCIP_CALL(SCIPcreateProbBasic(scip, "MLU_ILP_DCN")); // create the SCIP problem
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

  auto begin = std::chrono::high_resolution_clock::now();
  // Solve the problem
  SCIP_CALL(SCIPsolve(scip));
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Solved in "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(
              end - begin).count() << "ns" << std::endl;

  // Get the solutions
  SCIP_SOL *sol = nullptr;
  sol = SCIPgetBestSol(scip);

  scip_result_ = std::vector<std::vector<std::vector<double>>>(numSbPerDcn,
                                                               std::vector<std::vector<double>>(
                                                                 numSbPerDcn));
  std::cout << "problem result: " << SCIPgetSolVal(scip, sol, u) << std::endl;
  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb)
    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
      if (src_sb == dst_sb) continue;
//			std::cout << src_sb << "->" << dst_sb << ": ";
      for (int p : per_pair_paths_[src_sb][dst_sb]) {
//				std::cout << SCIPgetSolVal(scip, sol,
//				             x[src_sb][dst_sb][paths_[p].per_pair_id]) << ", ";
        scip_result_[src_sb][dst_sb].push_back(SCIPgetSolVal(scip, sol,
                                                             x[src_sb][dst_sb][paths_[p].per_pair_id]));
      }
//			std::cout << std::endl;
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

void FullTopology::ResultAnalysis() {
  // traffic amount of each link
  std::vector<double> links_load = std::vector<double>(links_.size());
  std::cout << "The path with 0 traffic is not printed. " << std::endl;
  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb)
    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb)
      if (src_sb != dst_sb) {
        for (int p : per_pair_paths_[src_sb][dst_sb]) {
          // print the traffic amount for each link
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
          // add the traffic to the links' load
          if (paths_[p].link_gid_list.size() == 1) {
            int first_hop = links_[paths_[p].link_gid_list.front()].gid;
            links_load[first_hop] += traffic_amount;
          }
          else if (paths_[p].link_gid_list.size() == 2) {
            int first_hop = links_[paths_[p].link_gid_list.front()].gid;
            int second_hop = links_[paths_[p].link_gid_list.back()].gid;
            links_load[first_hop] += traffic_amount;
            links_load[second_hop] += traffic_amount;
          }
        }
      }
  // print the link utilization
  for (int l=0; l < links_.size(); ++l) {
    std::cout << "Link " << l << ": "
              << links_load[l]/links_[l].capacity
              << std::endl;
  }
  // print the WCMP group weight at source s3 -> destination SB level
  std::unordered_map<int, std::vector<std::pair<int, double>>> assignment;
  std::unordered_map<int, std::vector<std::pair<int, double>>>::iterator it;
  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb)
    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb)
      if (src_sb != dst_sb) {
        for (int p : per_pair_paths_[src_sb][dst_sb]) {
          int src_sw = paths_[p].src_sw_gid;
          int key = src_sw*numSbPerDcn + dst_sb;
          int link_gid = paths_[p].link_gid_list.front();
          double traffic_amount =
            traffic_matrix_[src_sb * numSbPerDcn + dst_sb] *
            scip_result_[src_sb][dst_sb][paths_[p].per_pair_id];
          // add the traffic amount to the group
          it = assignment.find(key);
          if (it == assignment.end()) { // new key
            std::vector<std::pair<int, double>> new_vec;
            new_vec.emplace_back(std::make_pair(link_gid, traffic_amount));
            assignment[key] = new_vec;
          }
          else { // existing key
            assignment[key].emplace_back(std::make_pair(link_gid, traffic_amount));
          }
        }
      }

  // print the WCMP group that serves the original traffic from its own
  for (it=assignment.begin(); it!=assignment.end(); ++it) {
    // set group vector
    std::unordered_map<int, double> weights;
    int src_sw = it->first / numSbPerDcn;
    int dst_sb = it->first % numSbPerDcn;

    double sum_weight = 0;
    std::unordered_map<int, double>::iterator iter;
    for (std::pair<int, double> p : it->second) {
      int link = p.first;
      iter = weights.find(link);
      if (iter == weights.end()) {
        weights[link] = p.second;
        sum_weight += p.second;
      }
      else {
        weights[link] += p.second;
        sum_weight += p.second;
      }
    }
    if (sum_weight <= 0) continue;
    std::cout << "Group Assignment for Switch " << src_sw << " -> "
              << "SuperBlock " << dst_sb << " " << std::endl;
    for (iter=weights.begin(); iter!=weights.end(); ++iter) {
      std::cout << "link " << iter->first << ": " << iter->second/sum_weight*127 << std::endl;
    }
  }
}

} // namespace full
} // namespace topo
} // namespace wcmp