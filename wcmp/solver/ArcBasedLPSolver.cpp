//
// Created by wwt on 6/20/20.
//


#include <chrono>
#include <glog/logging.h>
#include "ArcBasedLPSolver.h"

namespace wcmp {
namespace solver {

ArcBasedLPSolver::ArcBasedLPSolver(wcmp::topo::AbstractTopology topology, std::unordered_map<int, double> &trace): AbstractSolver(topology, trace) {

}

// create variable for MLU
SCIP_RETCODE ArcBasedLPSolver::ArcLPCreateVariableGoal(SCIP *scip, SCIP_VAR *&y) {
  SCIP_CALL(SCIPcreateVarBasic(scip,
                               &y, // variable
                               "MLU", // name
                               0.0, // lower bound
                               infinity, // upper bound
                               1.0, // objective
                               SCIP_VARTYPE_CONTINUOUS)); // variable type
  SCIP_CALL(SCIPaddVar(scip, y));  //Adding the variable
  return SCIP_OKAY;
}

// create variable for weights
SCIP_RETCODE ArcBasedLPSolver::ArcLPCreateVariableF(SCIP *scip,
                                                std::vector<std::vector<std::vector<SCIP_VAR *>>> &f) {
  f = std::vector<std::vector<std::vector<SCIP_VAR *>>>(sources_.size(),
    std::vector<std::vector<SCIP_VAR *>>(destinations_.size()));
  for (int src_idx = 0; src_idx < sources_.size(); ++src_idx) {
    for (int dst_idx = 0; dst_idx < destinations_.size(); ++dst_idx) {
      if (sources_[src_idx] == destinations_[dst_idx]) continue;
      for (int l = 0; l < links_.size(); ++l) {
        f[src_idx][dst_idx].emplace_back((SCIP_VAR *) nullptr);
        std::stringstream ss;
        ss << "f_" << src_idx << "_" << dst_idx << "_" << l;
        SCIP_CALL(SCIPcreateVarBasic(scip,
                                     &f[src_idx][dst_idx][l], // variable
                                     ss.str().c_str(), // name
                                     0.0, // lower bound
                                     1.0, // upper bound
                                     0.0, // objective
                                     SCIP_VARTYPE_CONTINUOUS)); // variable type
        SCIP_CALL(SCIPaddVar(scip, f[src_idx][dst_idx][l]));  //Adding the variable
      }
    }
  }
  return SCIP_OKAY;
}

// add constraints for link constructive
// the input flow and the output flow should be the same
SCIP_RETCODE ArcBasedLPSolver::ArcLPCreateConstraints1(
  SCIP *scip,
  std::vector<SCIP_CONS *> &cons_1,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &f) {
  int cnt = 0;
  // iterate all src and dst
  for (int src_idx = 0; src_idx < sources_.size(); ++src_idx) {
    for (int dst_idx = 0; dst_idx < destinations_.size(); ++dst_idx) {
      if (sources_[src_idx] == destinations_[dst_idx]) continue;
      // iterate all mid switches
      for (int sw=0; sw<switches_.size(); ++sw) {
        if ((switches_[sw].switch_type == SwitchType::src) ||
        (switches_[sw].switch_type == SwitchType::dst)) continue;
        cons_1.emplace_back((SCIP_CONS *) nullptr); // add constraint
        std::vector<SCIP_VAR *> vars;
        std::vector<SCIP_Real> values;
        for (int l : per_switch_links_[sw]) {
          if (links_[l].src_sw_gid == sw) {
            vars.push_back(f[src_idx][dst_idx][l]);
            values.push_back(1);
          }
          else if (links_[l].dst_sw_gid == sw) {
            vars.push_back(f[src_idx][dst_idx][l]);
            values.push_back(-1);
          }
        }
        // add constraints
        SCIP_VAR *scip_vars[vars.size()];
        for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
        SCIP_Real scip_values[values.size()];
        for (int v = 0; v < values.size(); ++v) scip_values[v] = values[v];
        std::stringstream ss;
        ss << "cons_1_" << src_idx << "_" << dst_idx << "_" << sw;
        SCIP_CALL(SCIPcreateConsBasicLinear(scip,
                                            &cons_1[cnt], // constraint
                                            ss.str().c_str(), // name
                                            vars.size(), // how many variables
                                            scip_vars, // array of pointers to various variables
                                            scip_values, // array of values of the coefficients of corresponding variables
                                            0, // LHS of the constraint
                                            0)); // RHS of the constraint
        SCIP_CALL(SCIPaddCons(scip, cons_1[cnt]));
        ++cnt;
      }
    }
  }
  return SCIP_OKAY;
}

// add constraints for traffic sources and destinations
SCIP_RETCODE ArcBasedLPSolver::ArcLPCreateConstraints2(
  SCIP *scip,
  std::vector<SCIP_CONS *> &cons_2,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &f) {
  int cnt = 0;
  for (int src_idx = 0; src_idx < sources_.size(); ++src_idx) {
    for (int dst_idx = 0; dst_idx < destinations_.size(); ++dst_idx) {
      if (sources_[src_idx] == destinations_[dst_idx]) continue;
      // add constraints for source switches
      for (int vs=0; vs < sources_.size(); ++vs) {
        cons_2.emplace_back((SCIP_CONS *) nullptr); // add constraint
        std::vector<SCIP_VAR *> vars;
        std::vector<SCIP_Real> values;
        int vsw = sources_[vs];
        for (int i=0; i<per_switch_links_[vsw].size(); ++i) {
          int l = per_switch_links_[vsw][i];
          std::cout << i << " " << l << " " << src_idx;
          vars.push_back(f[src_idx][dst_idx][l]);
          values.push_back(1);
        }
        // add constraints
        SCIP_VAR *scip_vars[vars.size()];
        for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
        SCIP_Real scip_values[values.size()];
        for (int v = 0; v < values.size(); ++v) scip_values[v] = values[v];
        std::stringstream ss;
        ss << "cons_2_" << src_idx << "_" << dst_idx << "_" << vs;
        int lrhs;
        if (vs == src_idx) lrhs = 1;
        else lrhs = 0;
        SCIP_CALL(SCIPcreateConsBasicLinear(scip,
                                            &cons_2[cnt], // constraint
                                            ss.str().c_str(), // name
                                            vars.size(), // how many variables
                                            scip_vars, // array of pointers to various variables
                                            scip_values, // array of values of the coefficients of corresponding variables
                                            lrhs, // LHS of the constraint
                                            lrhs)); // RHS of the constraint
        SCIP_CALL(SCIPaddCons(scip, cons_2[cnt]));
        ++cnt;
      }
      // add constraints for destination switches
      for (int vs=0; vs < destinations_.size(); ++vs) {
        cons_2.emplace_back((SCIP_CONS *) nullptr); // add constraint
        std::vector<SCIP_VAR *> vars;
        std::vector<SCIP_Real> values;
        int vsw = destinations_[vs];
        for (int i=0; i<per_switch_links_[vsw].size(); ++i) {
          int l = per_switch_links_[vsw][i];
          vars.push_back(f[src_idx][dst_idx][l]);
          values.push_back(1);
        }
        // add constraints
        SCIP_VAR *scip_vars[vars.size()];
        for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
        SCIP_Real scip_values[values.size()];
        for (int v = 0; v < values.size(); ++v) scip_values[v] = values[v];
        std::stringstream ss;
        ss << "cons_2_" << src_idx << "_" << dst_idx << "_" << vs;
        int lrhs;
        if (vs == dst_idx) lrhs = 1;
        else lrhs = 0;
        SCIP_CALL(SCIPcreateConsBasicLinear(scip,
                                            &cons_2[cnt], // constraint
                                            ss.str().c_str(), // name
                                            vars.size(), // how many variables
                                            scip_vars, // array of pointers to various variables
                                            scip_values, // array of values of the coefficients of corresponding variables
                                            lrhs, // LHS of the constraint
                                            lrhs)); // RHS of the constraint
        SCIP_CALL(SCIPaddCons(scip, cons_2[cnt]));
        ++cnt;
      }
    }
  }
  return SCIP_OKAY;
}

// set the constraint: link utilization < u
SCIP_RETCODE ArcBasedLPSolver::ArcLPCreateConstraints9(
  SCIP *scip,
  std::vector<SCIP_CONS *> &cons_9,
  SCIP_VAR *&u,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &f) {
  int cnt = 0;
  // iterate all the links
  for (int i = 0; i < links_.size(); ++i) {
    if ((links_[i].link_type == LinkType::src) ||
        (links_[i].link_type == LinkType::dst)) continue;
    cons_9.emplace_back((SCIP_CONS *) nullptr); // add constraint
    std::vector<SCIP_VAR *> vars;
    std::vector<SCIP_Real> values;

    // iterate all the paths to check whether contain that link
    for (int src_idx=0; src_idx < sources_.size(); ++src_idx) {
      for (int dst_idx=0; dst_idx < destinations_.size(); ++dst_idx) {
        if (src_idx == dst_idx) continue;
        vars.push_back(f[src_idx][dst_idx][i]);
        values.push_back(-traffic_matrix_[src_idx * destinations_.size() + dst_idx]);
      }
    }

    SCIP_VAR *scip_vars[vars.size() + 1];
    for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
    SCIP_Real scip_values[values.size() + 1];
    for (int v = 0; v < values.size(); ++v) scip_values[v] = values[v];

    // add u
    scip_vars[vars.size()] = u;
    scip_values[vars.size()] = links_[i].capacity;

    std::stringstream ss;
    ss << "cons_9_" << links_[i].gid;
    SCIP_CALL(SCIPcreateConsBasicLinear(scip,
                                        &cons_9[cnt], // constraint
                                        ss.str().c_str(), // name
                                        vars.size()+1, // how many variables
                                        scip_vars, // array of pointers to various variables
                                        scip_values, // array of values of the coefficients of corresponding variables
                                        0, // LHS of the constraint
                                        infinity)); // RHS of the constraint
    SCIP_CALL(SCIPaddCons(scip, cons_9[cnt]));
    ++cnt;
  }
  return SCIP_OKAY;
}

// find the best routing policy in the DCN level
SCIP_RETCODE ArcBasedLPSolver::FindBestRouting() {
  SCIP *scip = nullptr;
  SCIP_CALL(SCIPcreate(&scip)); // create the SCIP environment

  SCIP_CALL(SCIPincludeDefaultPlugins(scip)); // include default plugins
  SCIP_CALL(SCIPcreateProbBasic(scip, "MLU_ILP_DCN")); // create the SCIP problem
  SCIP_CALL(SCIPsetObjsense(scip,
                            SCIP_OBJSENSE_MINIMIZE)); // set object sense to be minimize

  std::cout << "SCIP setup successfully" << std::endl;

  SCIP_RETCODE ret;

  SCIP_VAR *u; // MLU
  ret = ArcLPCreateVariableGoal(scip, u);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The variable u is wrong.";
  else std::cout << "Variable u created." << std::endl;

  std::vector<std::vector<std::vector<SCIP_VAR *>>> f; // initialize the variables
  ret = ArcLPCreateVariableF(scip, f);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The variable f is wrong.";
  else std::cout << "Variable f created." << std::endl;

  std::vector<SCIP_CONS *> cons_1;
  ret = ArcLPCreateConstraints1(scip, cons_1, f);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The equal constraints is wrong.";
  else std::cout << "Constraints 1 created." << std::endl;

  std::vector<SCIP_CONS *> cons_2;
  ret = ArcLPCreateConstraints2(scip, cons_2, f);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The link constraints is wrong.";
  else std::cout << "Constraints 2 created" << std::endl;

  std::vector<SCIP_CONS *> cons_9;
  ret = ArcLPCreateConstraints9(scip, cons_9, u, f);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The link constraints is wrong.";
  else std::cout << "Constraints 9 created" << std::endl;

  SCIP_CALL((SCIPwriteOrigProblem(scip, "MLU_before.lp", nullptr, FALSE)));

  // Release the constraints
  for (SCIP_CONS *con : cons_1) {
    SCIP_CALL(SCIPreleaseCons(scip, &con));
  }
  for (SCIP_CONS *con : cons_2) {
    SCIP_CALL(SCIPreleaseCons(scip, &con));
  }
  for (SCIP_CONS *con : cons_9) {
    SCIP_CALL(SCIPreleaseCons(scip, &con));
  }
  std::cout << "Constraints released" << std::endl;

  SCIP_CALL((SCIPwriteOrigProblem(scip, "MLU_before.lp", nullptr, FALSE)));
  auto begin = std::chrono::high_resolution_clock::now();
  // Solve the problem
  SCIP_CALL(SCIPsolve(scip));
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Solved in "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(
              end - begin).count() << "ns" << std::endl;

  // record the model
  SCIP_CALL((SCIPwriteOrigProblem(scip, "MLU.lp", nullptr, FALSE)));
  // Get the solutions
  SCIP_SOL *sol = nullptr;
  sol = SCIPgetBestSol(scip);

  res_f_ = std::vector<std::vector<std::vector<double>>>(sources_.size(),
    std::vector<std::vector<double>>(destinations_.size()));
  res_u_ = SCIPgetSolVal(scip, sol, u);
  std::cout << "problem result: " << res_u_ << std::endl;

  SCIP_CALL(SCIPreleaseVar(scip, &u));
  SCIP_CALL(SCIPfree(&scip));
  return SCIP_OKAY;
}

//void ArcBasedLPSolver::ArcLPResultAnalysis() {
//  // traffic amount of each link
//  std::vector<double> links_load = std::vector<double>(links_.size());
//  std::cout << "The path with 0 traffic is not printed. " << std::endl;
//  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb)
//    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb)
//      if (src_sb != dst_sb) {
//        for (int p : per_sb_pair_paths_[src_sb][dst_sb]) {
//          // print the traffic amount for each link
//          double traffic_amount =
//            traffic_matrix_[src_sb * numSbPerDcn + dst_sb] *
//            scip_result_[src_sb][dst_sb][paths_[p].per_pair_id];
//          if (traffic_amount > 0) {
//            std::cout << traffic_amount << " Gbps of demand from u"
//                      << src_sb << " -> u" << dst_sb
//                      << " is placed on DCN link "
//                      << links_[paths_[p].link_gid_list.front()].gid
//                      << std::endl;
//          }
//          // add the traffic to the links' load
//          if (paths_[p].link_gid_list.size() == 1) {
//            int first_hop = links_[paths_[p].link_gid_list.front()].gid;
//            links_load[first_hop] += traffic_amount;
//          }
//          else if (paths_[p].link_gid_list.size() == 2) {
//            int first_hop = links_[paths_[p].link_gid_list.front()].gid;
//            int second_hop = links_[paths_[p].link_gid_list.back()].gid;
//            links_load[first_hop] += traffic_amount;
//            links_load[second_hop] += traffic_amount;
//          }
//        }
//      }
//  // print the link utilization
//  for (int l=0; l < links_.size(); ++l) {
//    std::cout << "Link " << l << ": "
//              << links_load[l]/links_[l].capacity
//              << std::endl;
//  }
//  // print the WCMP group weight at source s3 -> destination SB level
//  std::unordered_map<int, std::vector<std::pair<int, double>>> assignment;
//  std::unordered_map<int, std::vector<std::pair<int, double>>>::iterator it;
//  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb)
//    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb)
//      if (src_sb != dst_sb) {
//        for (int p : per_sb_pair_paths_[src_sb][dst_sb]) {
//          int src_sw = paths_[p].src_sw_gid;
//          int key = src_sw*numSbPerDcn + dst_sb;
//          int link_gid = paths_[p].link_gid_list.front();
//          double traffic_amount =
//            traffic_matrix_[src_sb * numSbPerDcn + dst_sb] *
//            scip_result_[src_sb][dst_sb][paths_[p].per_pair_id];
//          // add the traffic amount to the group
//          it = assignment.find(key);
//          if (it == assignment.end()) { // new key
//            std::vector<std::pair<int, double>> new_vec;
//            new_vec.emplace_back(std::make_pair(link_gid, traffic_amount));
//            assignment[key] = new_vec;
//          }
//          else { // existing key
//            assignment[key].emplace_back(std::make_pair(link_gid, traffic_amount));
//          }
//        }
//      }
//
//  // print the WCMP group that serves the original traffic from its own
//  for (it=assignment.begin(); it!=assignment.end(); ++it) {
//    // set group vector
//    std::unordered_map<int, double> weights;
//    int src_sw = it->first / numSbPerDcn;
//    int dst_sb = it->first % numSbPerDcn;
//
//    double sum_weight = 0;
//    std::unordered_map<int, double>::iterator iter;
//    for (std::pair<int, double> p : it->second) {
//      int link = p.first;
//      iter = weights.find(link);
//      if (iter == weights.end()) {
//        weights[link] = p.second;
//        sum_weight += p.second;
//      }
//      else {
//        weights[link] += p.second;
//        sum_weight += p.second;
//      }
//    }
//    if (sum_weight <= 0) continue;
//    std::cout << "Group Assignment for Switch " << src_sw << " -> "
//              << "SuperBlock " << dst_sb << " " << std::endl;
//    for (iter=weights.begin(); iter!=weights.end(); ++iter) {
//      std::cout << "link " << iter->first << ": " << iter->second/sum_weight*127 << std::endl;
//    }
//  }
//}

} // namespace solver
} // namespace wcmp