//
// Created by wwt on 6/20/20.
//


#include <chrono>
#include <glog/logging.h>
#include "PathBasedLPSolver.h"

namespace wcmp {
namespace solver {

PathBasedLPSolver::PathBasedLPSolver(wcmp::topo::AbstractTopology topology, std::unordered_map<int, double> &trace): AbstractSolver(topology, trace) {

}

// create variable for MLU
SCIP_RETCODE PathBasedLPSolver::PathLPCreateVariableGoal(SCIP *scip, SCIP_VAR *&u) {
  SCIP_CALL(SCIPcreateVarBasic(scip,
                               &u, // variable
                               "MLU", // name
                               0.0, // lower bound
                               infinity, // upper bound
                               1.0, // objective
                               SCIP_VARTYPE_CONTINUOUS)); // variable type
  SCIP_CALL(SCIPaddVar(scip, u));  // Adding the variable
  return SCIP_OKAY;
}

// create variable for weights
SCIP_RETCODE PathBasedLPSolver::PathLPCreateVariableWeight(SCIP *scip,
                                                      std::vector<std::vector<std::vector<SCIP_VAR *>>> &x) {
  x = std::vector<std::vector<std::vector<SCIP_VAR *>>>(sources_.size(),
    std::vector<std::vector<SCIP_VAR *>>(destinations_.size()));
  for (int src_idx = 0; src_idx < sources_.size(); ++src_idx) {
    for (int dst_idx = 0; dst_idx < destinations_.size(); ++dst_idx) {
      for (int p = 0; p < per_pair_paths_[src_idx][dst_idx].size(); ++p) {
        x[src_idx][dst_idx].emplace_back((SCIP_VAR *) nullptr);
        std::stringstream ss;
        ss << "x_" << src_idx << "_" << dst_idx << "_" << p;
        SCIP_CALL(SCIPcreateVarBasic(scip,
                                     &x[src_idx][dst_idx][p], // variable
                                     ss.str().c_str(), // name
                                     0.0, // lower bound
                                     1.0, // upper bound
                                     0.0, // objective
                                     SCIP_VARTYPE_CONTINUOUS)); // variable type
        SCIP_CALL(SCIPaddVar(scip,
                             x[src_idx][dst_idx][p]));  //Adding the variable
      }
    }
  }
  return SCIP_OKAY;
}

// add constraints for Sum(x_{ij})=1
SCIP_RETCODE PathBasedLPSolver::PathLPCreateConstraintsEqualToOne(
  SCIP *scip,
  std::vector<SCIP_CONS *> &equal_cons,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &x) {
  int cnt = 0;
  for (int src_idx = 0; src_idx < sources_.size(); ++src_idx) {
    for (int dst_idx = 0; dst_idx < destinations_.size(); ++dst_idx) {
      if (src_idx == dst_idx) continue; // skip the self loop
      equal_cons.emplace_back((SCIP_CONS *) nullptr); // add constraint
      int num_paths = per_pair_paths_[src_idx][dst_idx].size();
      SCIP_Real values[num_paths];
      for (int k = 0; k < num_paths; ++k) values[k] = 1.0;
      std::stringstream ss;
      ss << "cons_equal_" << src_idx << "_" << dst_idx;
      SCIP_CALL(SCIPcreateConsBasicLinear(scip,
                                          &equal_cons[cnt], // constraint
                                          ss.str().c_str(), // name
                                          num_paths, // how many variables
                                          &x[src_idx][dst_idx][0], // array of pointers to various variables
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
SCIP_RETCODE PathBasedLPSolver::PathLPCreateConstraintsLinkUtilizationBound(
  SCIP *scip,
  std::vector<SCIP_CONS *> &equal_cons,
  SCIP_VAR *&u,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &x) {
  // iterate all the links
  for (int i = 0; i < links_.size(); ++i) {
    equal_cons.emplace_back((SCIP_CONS *) nullptr); // add constraint
    std::vector<SCIP_VAR *> vars;
    std::vector<SCIP_Real> values;

    // iterate all the paths to check whether contain that link
    for (int path_gid : per_link_paths_[i]) {
      int src_idx = paths_[path_gid].source_id;
      int dst_idx = paths_[path_gid].destination_id;
      vars.push_back(x[src_idx][dst_idx][paths_[path_gid].per_pair_id]);
      values.push_back(traffic_matrix_[src_idx * destinations_.size() + dst_idx] /
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
                                        &equal_cons[i], // constraint
                                        ss.str().c_str(), // name
                                        vars.size() +
                                        1, // how many variables
                                        scip_vars, // array of pointers to various variables
                                        scip_values, // array of values of the coefficients of corresponding variables
                                        0, // LHS of the constraint
                                        infinity)); // RHS of the constraint
    SCIP_CALL(SCIPaddCons(scip, equal_cons[i]));
  }
  return SCIP_OKAY;
}

// find the best routing policy
SCIP_RETCODE PathBasedLPSolver::FindBestRouting() {
  SCIP *scip = nullptr;
  SCIP_CALL(SCIPcreate(&scip)); // create the SCIP environment

  SCIP_CALL(SCIPincludeDefaultPlugins(scip)); // include default plugins
  SCIP_CALL(SCIPcreateProbBasic(scip, "MLU_ILP_DCN")); // create the SCIP problem
  SCIP_CALL(SCIPsetObjsense(scip,
                            SCIP_OBJSENSE_MINIMIZE)); // set object sense to be minimize

  std::cout << "SCIP setup successfully" << std::endl;

  SCIP_RETCODE ret;

  SCIP_VAR *u; // MLU
  ret = PathLPCreateVariableGoal(scip, u);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The variable u is wrong.";
  else std::cout << "Variable u created." << std::endl;

  std::vector<std::vector<std::vector<SCIP_VAR *>>> x; // initialize the variables
  ret = PathLPCreateVariableWeight(scip, x);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The variable x is wrong.";
  else std::cout << "Variable x created." << std::endl;

  std::vector<SCIP_CONS *> equal_cons;
  ret = PathLPCreateConstraintsEqualToOne(scip, equal_cons, x);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The equal constraints is wrong.";
  else std::cout << "Equal constraints created." << std::endl;

  std::vector<SCIP_CONS *> link_cons;
  ret = PathLPCreateConstraintsLinkUtilizationBound(scip, link_cons, u, x);
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

  scip_result_ = std::vector<std::vector<std::vector<double>>>(sources_.size(),
    std::vector<std::vector<double>>(destinations_.size()));
  std::cout << "problem result: " << SCIPgetSolVal(scip, sol, u) << std::endl;
  for (int src_idx = 0; src_idx < sources_.size(); ++src_idx)
    for (int dst_idx = 0; dst_idx < destinations_.size(); ++dst_idx) {
      if (src_idx == dst_idx) continue;
//			std::cout << src_idx << "->" << dst_idx << ": ";
      for (int p : per_pair_paths_[src_idx][dst_idx]) {
//				std::cout << SCIPgetSolVal(scip, sol,
//				             x[src_idx][dst_idx][paths_[p].per_pair_id]) << ", ";
        scip_result_[src_idx][dst_idx].push_back(SCIPgetSolVal(scip, sol,
                                                               x[src_idx][dst_idx][paths_[p].per_pair_id]));
      }
//			std::cout << std::endl;
    }

  SCIP_CALL((SCIPwriteOrigProblem(scip, "MLU.lp", nullptr, FALSE)));
  for (int src_idx = 0; src_idx < sources_.size(); ++src_idx) {
    for (int dst_idx = 0; dst_idx < destinations_.size(); ++dst_idx) {
      for (SCIP_VAR *v : x[src_idx][dst_idx])
        SCIP_CALL(SCIPreleaseVar(scip, &v));
    }
  }
  SCIP_CALL(SCIPreleaseVar(scip, &u));
  SCIP_CALL(SCIPfree(&scip));
  return SCIP_OKAY;
}

// get the group memory occupation
int GroupSize(const std::vector<int> &weights) {
  int total_size = 0;
  for (const int weight : weights) {
    total_size += weight;
  }
  return total_size;
}

// helper function for weight reduction
double CalcDelta(std::vector<int> &weights, std::vector<int> &new_weights) {
  double max_delta = 0;
  int total_size = GroupSize(weights);
  int new_total_size = GroupSize(new_weights);
  for (int i = 0; i < weights.size(); ++i) {
    if (weights[i] == 0) continue;
    double delta = new_weights[i] * total_size / (weights[i] * new_total_size);
    if (delta > max_delta) {
      max_delta = delta;
    }
  }
  return max_delta;
}

// helper function for weight reduction
int ChoosePortToUpdate(std::vector<int> &weights, std::vector<int> &new_weights) {
  double min_oversub = wcmp::infinity;
  int index = -1;
  int total_size = GroupSize(weights);
  int new_total_size = GroupSize(new_weights);
  for (int i = 0; i < weights.size(); ++i) {
    double oversub = (new_weights[i] + 1) * total_size /
                     double((new_total_size + 1) * weights[i]);
    if (oversub < min_oversub) {
      min_oversub = oversub;
      index = i;
    }
  }
  return index;
}

// the weight reduction function used in WCMP (eurosys)
std::vector<int> EurosysMethod(std::vector<int> weights) {
  std::vector<int> new_weights;
  for (const int weight : weights) {
    new_weights.emplace_back(weight);
  }
  int total_size = GroupSize(new_weights);
  while (total_size > maxGroupSize) {
    int non_reducible_size = 0;
    for (int i = 0; i < new_weights.size(); ++i) {
      if (new_weights[i] == 1) {
        non_reducible_size += new_weights[i];
      }
    }
    if (non_reducible_size == new_weights.size()) {
      break;
    }
    double reduction_ratio =
      (maxGroupSize - non_reducible_size) / double(GroupSize(weights));
    for (int i = 0; i < new_weights.size(); ++i) {
      new_weights[i] = int(weights[i] * reduction_ratio);
      if (new_weights[i] == 0) {
        new_weights[i] = 1;
      }
    }
    total_size = GroupSize(new_weights);
  }
  std::vector<int> results;
  for (const int weight : new_weights) {
    results.push_back(weight);
  }
  int remaining_size = maxGroupSize - GroupSize(new_weights);
  double min_oversub = CalcDelta(weights, new_weights);
  for (int k = 0; k < remaining_size; ++k) {
    int index = ChoosePortToUpdate(weights, new_weights);
    new_weights[index] += 1;
    if (min_oversub > CalcDelta(weights, new_weights)) {
      for (int i = 0; i < new_weights.size(); ++i) {
        results[i] = new_weights[i];
      }
      min_oversub = CalcDelta(weights, new_weights);
    }
  }
  return results;
}

void PathBasedLPSolver::ResultAnalysis() {
  // traffic amount of each link
  std::vector<double> links_load = std::vector<double>(links_.size());
  std::cout << "The path with 0 traffic is not printed. " << std::endl;
  for (int src_sb = 0; src_sb < sources_.size(); ++src_sb)
    for (int dst_sb = 0; dst_sb < destinations_.size(); ++dst_sb)
      if (src_sb != dst_sb) {
        for (int p : per_pair_paths_[src_sb][dst_sb]) {
          // print the traffic amount for each link
          double traffic_amount =
            traffic_matrix_[src_sb * destinations_.size() + dst_sb] *
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
  for (int l=0; l < links_load.size(); ++l) {
    std::cout << "Link " << l << ": "
              << links_load[l]/links_[l].capacity
              << std::endl;
  }
  // print the WCMP group weight at source s3 -> destination SB level
  std::unordered_map<int, std::vector<std::pair<int, double>>> assignment;
  std::unordered_map<int, std::vector<std::pair<int, double>>>::iterator it;
  for (int src_sb = 0; src_sb < sources_.size(); ++src_sb)
    for (int dst_sb = 0; dst_sb < destinations_.size(); ++dst_sb)
      if (src_sb != dst_sb) {
        for (int p : per_pair_paths_[src_sb][dst_sb]) {
          int src_sw = paths_[p].source_id;
          int key = src_sw*destinations_.size() + dst_sb;
          int link_gid = paths_[p].link_gid_list.front();
          double traffic_amount =
            traffic_matrix_[src_sb * destinations_.size() + dst_sb] *
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
    int src_sw = it->first / destinations_.size();
    int dst_sb = it->first % destinations_.size();

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
    std::vector<int> double_weight;
    std::vector<int> integer_weight;
    for (iter=weights.begin(); iter!=weights.end(); ++iter) {
      std::cout << "link " << iter->first << ": " << iter->second/sum_weight*127 << std::endl;
      double_weight.push_back(round(iter->second*1000));
    }
    integer_weight = EurosysMethod(double_weight);
    int cnt = 0;
    std::cout << "After Weight Reduction" << std::endl;
    for (iter=weights.begin(); iter!=weights.end(); ++iter) {
      std::cout << "    link " << iter->first << ": " << integer_weight[cnt] << std::endl;
      ++cnt;
    }
  }
}

} // namespace solver
} // namespace wcmp