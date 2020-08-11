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
SCIP_RETCODE FullTopology::ArcILPCreateVariableGoal(SCIP *scip, SCIP_VAR *&u) {
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
SCIP_RETCODE FullTopology::ArcILPCreateVariableF(SCIP *scip,
                                                 std::vector<std::vector<std::vector<SCIP_VAR *>>> &f) {
  f = std::vector<std::vector<std::vector<SCIP_VAR *>>>(numSbPerDcn,
    std::vector<std::vector<SCIP_VAR *>>(numSbPerDcn));
  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb) {
    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
      if (src_sb == dst_sb) continue;
      for (int l=0; l<links_.size(); ++l) {
        f[src_sb][dst_sb].emplace_back((SCIP_VAR *) nullptr);
        std::stringstream ss;
        ss << "f_" << src_sb << "_" << dst_sb << "_" << l;
        SCIP_CALL(SCIPcreateVarBasic(scip,
                                     &f[src_sb][dst_sb][l], // variable
                                     ss.str().c_str(), // name
                                     0.0, // lower bound
                                     1.0, // upper bound
                                     0.0, // objective
                                     SCIP_VARTYPE_CONTINUOUS)); // variable type
        SCIP_CALL(SCIPaddVar(scip, f[src_sb][dst_sb][l]));  //Adding the variable
      }
    }
  }
  return SCIP_OKAY;
}

// create variable for x
SCIP_RETCODE FullTopology::ArcILPCreateVariableX(SCIP *scip,
                                                 std::vector<std::vector<std::vector<SCIP_VAR *>>> &x) {
  x = std::vector<std::vector<std::vector<SCIP_VAR *>>>(numSbPerDcn,
    std::vector<std::vector<SCIP_VAR *>>(maxGroupSize+1));
  for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
    // get the Dt
    double dt = 0;
    for (int src_sb=0; src_sb<numSbPerDcn; ++src_sb) {
      if (src_sb!=dst_sb) dt = dt + traffic_matrix_[src_sb * numSbPerDcn + dst_sb];
    }
    for (int n=0; n<maxGroupSize+1; ++n) {
      for (int l=0; l<links_.size(); ++l) {
        if (links_[l].link_type == LinkType::dcn) {
          x[dst_sb][n].emplace_back((SCIP_VAR *) nullptr);
          std::stringstream ss;
          ss << "x_" << dst_sb << "_" << n << "_" << l;
          SCIP_CALL(SCIPcreateVarBasic(scip,
                                       &x[dst_sb][n][l], // variable
                                       ss.str().c_str(), // name
                                       -dt, // lower bound
                                       dt, // upper bound
                                       0.0, // objective
                                       SCIP_VARTYPE_CONTINUOUS)); // variable type
          SCIP_CALL(SCIPaddVar(scip, x[dst_sb][n][l]));  //Adding the variable
        }
      }
    }
  }
  return SCIP_OKAY;
}

// create variable for y
SCIP_RETCODE FullTopology::ArcILPCreateVariableY(SCIP *scip,
                                                 std::vector<std::vector<std::vector<SCIP_VAR *>>> &y) {
  y = std::vector<std::vector<std::vector<SCIP_VAR *>>>(numSbPerDcn,
    std::vector<std::vector<SCIP_VAR *>>(maxGroupSize+1));
  for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
    for (int n=0; n<maxGroupSize+1; ++n) {
      for (int l : link_group) {
        y[dst_sb][n].emplace_back((SCIP_VAR *) nullptr);
        std::stringstream ss;
        ss << "y_" << dst_sb << "_" << n << "_" << l;
        SCIP_CALL(SCIPcreateVarBasic(scip,
                                     &y[dst_sb][n][l], // variable
                                     ss.str().c_str(), // name
                                     0, // lower bound
                                     1, // upper bound
                                     0.0, // objective
                                     SCIP_VARTYPE_INTEGER)); // variable type
        SCIP_CALL(SCIPaddVar(scip, y[dst_sb][n][l]));  //Adding the variable
      }
    }
  }
  return SCIP_OKAY;
}

// create variable for b
SCIP_RETCODE FullTopology::ArcILPCreateVariableB(SCIP *scip,
                                                 std::vector<std::vector<SCIP_VAR *>> &b) {
  b = std::vector<std::vector<SCIP_VAR *>>(numSbPerDcn);
  for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
    // get the Dt
    double dt = 0;
    for (int src_sb=0; src_sb<numSbPerDcn; ++src_sb) {
      if (src_sb!=dst_sb) dt = dt + traffic_matrix_[src_sb * numSbPerDcn + dst_sb];
    }
    for (int sw=0; sw<switches_.size(); ++sw) {
      b[dst_sb].emplace_back((SCIP_VAR *) nullptr);
      std::stringstream ss;
      ss << "b_" << dst_sb << "_" << sw;
      SCIP_CALL(SCIPcreateVarBasic(scip,
                                   &b[dst_sb][sw], // variable
                                   ss.str().c_str(), // name
                                   0, // lower bound
                                   1, // upper bound
                                   0.0, // objective
                                   SCIP_VARTYPE_INTEGER)); // variable type
      SCIP_CALL(SCIPaddVar(scip, b[dst_sb][sw]));  //Adding the variable
    }
  }
  return SCIP_OKAY;
}

// add constraints for (1)
// the input flow and the output flow should be the same
SCIP_RETCODE FullTopology::ArcILPCreateConstraints1(
  SCIP *scip,
  std::vector<SCIP_CONS *> &cons_1,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &f) {
  int cnt = 0;
  // iterate all src and dst
  for (int src_sb = 0; src_sb < src_sw_group.size(); ++src_sb) {
    for (int dst_sb = 0; dst_sb < dst_sw_group.size(); ++dst_sb) {
      if ((src_sb == dst_sb) && (wcmp::problem_scope<=1)) continue;
      // iterate all mid switches
      for (int sw : sw_group) {
        cons_1.emplace_back((SCIP_CONS *) nullptr); // add constraint
        std::vector<SCIP_VAR *> vars;
        std::vector<SCIP_Real> values;
        for (int l : per_switch_links_[sw]) {
          // only target the l in the link_group
          if (std::find(link_group.begin(), link_group.end(), l) != link_group.end()) {
            if (links_[l].src_sw_gid == sw) {
              vars.push_back(f[src_sb][dst_sb][l]);
              values.push_back(1);
            }
            else if (links_[l].dst_sw_gid == sw) {
              vars.push_back(f[src_sb][dst_sb][l]);
              values.push_back(-1);
            }
          }
        }
        // add constraints
        SCIP_VAR *scip_vars[vars.size()];
        for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
        SCIP_Real scip_values[values.size()];
        for (int v = 0; v < values.size(); ++v) scip_values[v] = values[v];
        std::stringstream ss;
        ss << "cons_1_" << src_sb << "_" << dst_sb << "_" << sw;
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

// add constraints for (2)
SCIP_RETCODE FullTopology::ArcILPCreateConstraints2(
  SCIP *scip,
  std::vector<SCIP_CONS *> &cons_2,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &f) {
  int cnt = 0;
  for (int src_idx = 0; src_idx < src_sw_group.size(); ++src_idx) {
    for (int dst_idx = 0; dst_idx < dst_sw_group.size(); ++dst_idx) {
      if ((src_idx == dst_idx) && (wcmp::problem_scope <= 1)) continue;
      int src_sw = src_sw_group[src_idx];
      int dst_sw = dst_sw_group[dst_idx];
      for (int sw : sw_group) {
        cons_2.emplace_back((SCIP_CONS *) nullptr); // add constraint
        std::vector<SCIP_VAR *> vars;
        std::vector<SCIP_Real> values;
        // source switch
        if (sw == src_sw) {
          for (int i=0; i<per_switch_links_[sw].size(); ++i) {
            int l = per_switch_links_[sw][i];
            if (links_[l].src_sw_gid == sw) {
              vars.push_back(f[src_idx][dst_idx][l]);
              values.push_back(1);
            }
          }
        }
        // destination switch
        else if (sw == dst_sw) {
          for (int i=0; i<per_switch_links_[sw].size(); ++i) {
            int l = per_switch_links_[sw][i];
            if (links_[l].dst_sw_gid == sw) {
              vars.push_back(f[src_idx][dst_idx][l]);
              values.push_back(1);
            }
          }
        }
        // other switches
        else {
          for (int i=0; i<per_switch_links_[sw].size(); ++i) {
            int l = per_switch_links_[sw][i];
            // only target the l in the link_group
            if (std::find(link_group.begin(), link_group.end(), l) != link_group.end()) {
              if (links_[l].src_sw_gid == sw) {
                vars.push_back(f[src_idx][dst_idx][l]);
                values.push_back(1);
              }
              else if (links_[l].dst_sw_gid == sw) {
                vars.push_back(f[src_idx][dst_idx][l]);
                values.push_back(-1);
              }
            }
          }
        }
        // add constraints
        SCIP_VAR *scip_vars[vars.size()];
        for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
        SCIP_Real scip_values[values.size()];
        for (int v = 0; v < values.size(); ++v) scip_values[v] = values[v];
        std::stringstream ss;
        ss << "cons_2_" << src_idx << "_" << dst_idx << "_" << sw;
        int lrhs;
        if ((sw == src_sw) || (sw == dst_sw)) lrhs = 1;
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

// add constraints for (3)
// integer requirement for the split
SCIP_RETCODE FullTopology::ArcILPCreateConstraints3(
  SCIP *scip,
  std::vector<SCIP_CONS *> &cons_3,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &f,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &x) {
  int cnt = 0;
  for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
    for (int l : link_group) {
      if (links_[l].link_type != LinkType::dcn) continue;
      for (int n = 0; n <= maxGroupSize; ++n) {
        cons_3.emplace_back((SCIP_CONS *) nullptr); // add constraint
        std::vector<SCIP_VAR *> vars;
        std::vector<SCIP_Real> values;
        for (int src_sb=0; src_sb<numSbPerDcn; ++src_sb) {
          if (src_sb==dst_sb) continue;
          vars.push_back(f[src_sb][dst_sb][l]);
          values.push_back(-traffic_matrix_[src_sb*numSbPerDcn+dst_sb]);
        }
        for (int pi=0; pi<per_switch_links_[links_[l].src_sw_gid].size(); ++pi) {
          int pl = per_switch_links_[links_[l].src_sw_gid][pi];
          if (links_[pl].dst_sw_gid != links_[l].src_sw_gid) continue;
          for (int src_sb=0; src_sb<numSbPerDcn; ++src_sb) {
            if (src_sb==dst_sb) continue;
            vars.push_back(f[src_sb][dst_sb][pl]);
            values.push_back(traffic_matrix_[src_sb*numSbPerDcn+dst_sb]*n/double(maxGroupSize));
          }
        }
        vars.push_back(x[dst_sb][n][l]);
        values.push_back(1);
        // add constraints
        SCIP_VAR *scip_vars[vars.size()];
        for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
        SCIP_Real scip_values[values.size()];
        for (int v = 0; v < values.size(); ++v) scip_values[v] = values[v];
        std::stringstream ss;
        ss << "cons_3_" << dst_sb << "_" << n << "_" << l;
        SCIP_CALL(SCIPcreateConsBasicLinear(scip,
                                            &cons_3[cnt], // constraint
                                            ss.str().c_str(), // name
                                            vars.size(), // how many variables
                                            scip_vars, // array of pointers to various variables
                                            scip_values, // array of values of the coefficients of corresponding variables
                                            0, // LHS of the constraint
                                            0)); // RHS of the constraint
        SCIP_CALL(SCIPaddCons(scip, cons_3[cnt]));
        ++cnt;
      }
    }
  }
  return SCIP_OKAY;
}

// add constraints for (4)
SCIP_RETCODE FullTopology::ArcILPCreateConstraints4(
  SCIP *scip,
  std::vector<SCIP_CONS *> &cons_4,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &x,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &y) {
  int cnt = 0;
  for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
    for (int l = 0; l < links_.size(); ++l) {
      if (links_[l].link_type != LinkType::dcn) continue;
      for (int n = 0; n <= maxGroupSize; ++n) {
        cons_4.emplace_back((SCIP_CONS *) nullptr); // add constraint
        std::vector<SCIP_VAR *> vars;
        std::vector<SCIP_Real> values;
        vars.push_back(x[dst_sb][n][l]);
        values.push_back(1);
        vars.push_back(y[dst_sb][n][l]);
        double dt = 0;
        for (int src_sb=0; src_sb<numSbPerDcn; ++src_sb) {
          dt+=traffic_matrix_[src_sb*numSbPerDcn+dst_sb];
        }
        values.push_back(-dt);
        // add constraints
        SCIP_VAR *scip_vars[vars.size()];
        for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
        SCIP_Real scip_values[values.size()];
        for (int v = 0; v < values.size(); ++v) scip_values[v] = values[v];
        std::stringstream ss;
        ss << "cons_4_" << dst_sb << "_" << n << "_" << l;
        SCIP_CALL(SCIPcreateConsBasicLinear(scip,
                                            &cons_4[cnt], // constraint
                                            ss.str().c_str(), // name
                                            vars.size(), // how many variables
                                            scip_vars, // array of pointers to various variables
                                            scip_values, // array of values of the coefficients of corresponding variables
                                            -dt, // LHS of the constraint
                                            infinity)); // RHS of the constraint
        SCIP_CALL(SCIPaddCons(scip, cons_4[cnt]));
        ++cnt;

        cons_4.emplace_back((SCIP_CONS *) nullptr); // add constraint
        std::vector<SCIP_VAR *> vars2;
        std::vector<SCIP_Real> values2;
        vars2.push_back(x[dst_sb][n][l]);
        values2.push_back(1);
        vars2.push_back(y[dst_sb][n][l]);
        values2.push_back(dt);
        // add constraints
        SCIP_VAR *scip_vars2[vars.size()];
        for (int v = 0; v < vars2.size(); ++v) scip_vars2[v] = vars2[v];
        SCIP_Real scip_values2[values2.size()];
        for (int v = 0; v < values2.size(); ++v) scip_values2[v] = values2[v];
        std::stringstream ss2;
        ss2 << "cons_4_" << dst_sb << "_" << n << "_" << l;
        SCIP_CALL(SCIPcreateConsBasicLinear(scip,
                                            &cons_4[cnt], // constraint
                                            ss2.str().c_str(), // name
                                            vars2.size(), // how many variables
                                            scip_vars2, // array of pointers to various variables
                                            scip_values2, // array of values of the coefficients of corresponding variables
                                            -infinity, // LHS of the constraint
                                            dt)); // RHS of the constraint
        SCIP_CALL(SCIPaddCons(scip, cons_4[cnt]));
        ++cnt;
      }
    }
  }
  return SCIP_OKAY;
}

// add constraints for (5)
SCIP_RETCODE FullTopology::ArcILPCreateConstraints5(
  SCIP *scip,
  std::vector<SCIP_CONS *> &cons_5,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &y,
  std::vector<std::vector<SCIP_VAR *>> &b) {
  int cnt = 0;
  for (int dst_idx = 0; dst_idx < dst_sw_group.size(); ++dst_idx) {
    for (int sw : int_sw_group) {
      for (int l : per_switch_links_[sw]) {
        if (std::find(link_group.begin(), link_group.end(), l) == link_group.end())
          continue;
        if (links_[l].dst_sw_gid == sw) continue;
        if (links_[l].link_type != LinkType::dcn) continue;
        std::cout << dst_idx << " " << sw << " " << l << std::endl;
        cons_5.emplace_back((SCIP_CONS *) nullptr); // add constraint
        std::vector<SCIP_VAR *> vars;
        std::vector<SCIP_Real> values;
        for (int n=0; n<=maxGroupSize; ++n) {
          vars.push_back(y[dst_idx][n][l]);
          values.push_back(1);
        }
        std::cout << "mid" << std::endl;
        vars.push_back(b[dst_idx][sw]);
        values.push_back(-1);
        // add constraints
        SCIP_VAR *scip_vars[vars.size()];
        for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
        SCIP_Real scip_values[values.size()];
        for (int v = 0; v < values.size(); ++v) scip_values[v] = values[v];
        std::stringstream ss;
        ss << "cons_5_" << dst_idx << "_" << l;
        std::cout << "set" << std::endl;
        SCIP_CALL(SCIPcreateConsBasicLinear(scip,
                                            &cons_5[cnt], // constraint
                                            ss.str().c_str(), // name
                                            vars.size(), // how many variables
                                            scip_vars, // array of pointers to various variables
                                            scip_values, // array of values of the coefficients of corresponding variables
                                            0, // LHS of the constraint
                                            0)); // RHS of the constraint
        SCIP_CALL(SCIPaddCons(scip, cons_5[cnt]));
        ++cnt;
        std::cout << "end" << std::endl;
      }
    }
  }
  return SCIP_OKAY;
}

// add constraints for (6)
SCIP_RETCODE FullTopology::ArcILPCreateConstraints6(
  SCIP *scip,
  std::vector<SCIP_CONS *> &cons_6,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &f,
  std::vector<std::vector<SCIP_VAR *>> &b) {
  int cnt = 0;
  for (int dst_idx = 0; dst_idx < dst_sw_group.size(); ++dst_idx) {
    for (int sw : int_sw_group) {
      cons_6.emplace_back((SCIP_CONS *) nullptr); // add constraint
      std::vector<SCIP_VAR *> vars;
      std::vector<SCIP_Real> values;
      std::cout << sw << " " << dst_idx << std::endl;
      for (int l : per_switch_links_[sw]) {
        if (std::find(link_group.begin(), link_group.end(), l) == link_group.end())
          continue;
        if (links_[l].dst_sw_gid == sw) {
          std::cout << l << " ";
          for (int src_idx = 0; src_idx < src_sw_group.size(); ++src_idx) {
            if ((src_idx == dst_idx) && (wcmp::problem_scope<=1)) continue;
            vars.push_back(f[src_idx][dst_idx][l]);
            values.push_back(-traffic_matrix_[src_idx * numSbPerDcn + dst_idx]);
          }
        }
      }
      vars.push_back(b[dst_idx][sw]);
      values.push_back(0.0);
      for (int src_idx = 0; src_idx < src_sw_group.size(); ++src_idx) {
        if ((src_idx == dst_idx) && (wcmp::problem_scope<=1)) continue;
        values[values.size()-1] += traffic_matrix_[src_idx * numSbPerDcn + dst_idx];
      }
      // add constraints
      for (int k =0; k<vars.size(); ++k) std::cout << values[k] << std::endl;
      SCIP_VAR *scip_vars[vars.size()];
      for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
      SCIP_Real scip_values[values.size()];
      for (int v = 0; v < values.size(); ++v) scip_values[v] = values[v];
      std::stringstream ss;
      SCIP_CALL(SCIPcreateConsBasicLinear(scip,
                                          &cons_6[cnt], // constraint
                                          ss.str().c_str(), // name
                                          vars.size(), // how many variables
                                          scip_vars, // array of pointers to various variables
                                          scip_values, // array of values of the coefficients of corresponding variables
                                          0, // LHS of the constraint
                                          infinity)); // RHS of the constraint
      SCIP_CALL(SCIPaddCons(scip, cons_6[cnt]));
      ++cnt;
    }
  }
  return SCIP_OKAY;
}

// set the constraint: link utilization < u
SCIP_RETCODE FullTopology::ArcILPCreateConstraints9(
  SCIP *scip,
  std::vector<SCIP_CONS *> &cons_9,
  SCIP_VAR *&u,
  std::vector<std::vector<std::vector<SCIP_VAR *>>> &f) {
  int cnt = 0;
  // iterate all the links
  for (int l : link_group) {
    cons_9.emplace_back((SCIP_CONS *) nullptr); // add constraint
    std::vector<SCIP_VAR *> vars;
    std::vector<SCIP_Real> values;

    // iterate all the paths to check whether contain that link
    for (int src_idx=0; src_idx < src_sw_group.size(); ++src_idx) {
      for (int dst_idx=0; dst_idx < dst_sw_group.size(); ++dst_idx) {
        if ((src_idx == dst_idx) && (wcmp::problem_scope<=1)) continue;
        std::cout << src_idx << " " << dst_idx << " " << l << std::endl;
        vars.push_back(f[src_idx][dst_idx][l]);
        values.push_back(-traffic_matrix_[src_idx * numSbPerDcn + dst_idx]);
      }
    }

    SCIP_VAR *scip_vars[vars.size() + 1];
    for (int v = 0; v < vars.size(); ++v) scip_vars[v] = vars[v];
    SCIP_Real scip_values[values.size() + 1];
    for (int v = 0; v < values.size(); ++v) scip_values[v] = values[v];

    // add u
    scip_vars[vars.size()] = u;
    scip_values[vars.size()] = links_[l].capacity;

    std::stringstream ss;
    ss << "cons_9_" << links_[l].gid;
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

// find the best routing policy in the AbstractTopology level
SCIP_RETCODE FullTopology::FindBestDcnRoutingArcILP() {
  SCIP *scip = nullptr;
  SCIP_CALL(SCIPcreate(&scip)); // create the SCIP environment

  SCIP_CALL(SCIPincludeDefaultPlugins(scip)); // include default plugins
  SCIP_CALL(SCIPcreateProbBasic(scip, "MLU_ILP_DCN")); // create the SCIP problem
  SCIP_CALL(SCIPsetObjsense(scip,
                            SCIP_OBJSENSE_MINIMIZE)); // set object sense to be minimize

  std::cout << "SCIP setup successfully" << std::endl;

  SCIP_RETCODE ret;

  SCIP_VAR *u; // MLU
  ret = ArcILPCreateVariableGoal(scip, u);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The variable u is wrong.";
  else std::cout << "Variable u created." << std::endl;

  std::vector<std::vector<std::vector<SCIP_VAR *>>> f; // initialize the variables
  ret = ArcILPCreateVariableF(scip, f);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The variable f is wrong.";
  else std::cout << "Variable f created." << std::endl;

  std::vector<std::vector<std::vector<SCIP_VAR *>>> x; // initialize the variables
  ret = ArcILPCreateVariableX(scip, x);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The variable x is wrong.";
  else std::cout << "Variable x created." << std::endl;

  std::vector<std::vector<std::vector<SCIP_VAR *>>> y; // initialize the variables
  ret = ArcILPCreateVariableY(scip, y);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The variable y is wrong.";
  else std::cout << "Variable y created." << std::endl;

  std::vector<std::vector<SCIP_VAR *>> b; // initialize the variables
  ret = ArcILPCreateVariableB(scip, b);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The variable b is wrong.";
  else std::cout << "Variable b created." << std::endl;

//  std::vector<SCIP_CONS *> cons_1;
//  ret = ArcILPCreateConstraints1(scip, cons_1, f);
//  if (ret != SCIP_OKAY) LOG(ERROR) << "The equal constraints is wrong.";
//  else std::cout << "Constraints 1 created." << std::endl;

  std::vector<SCIP_CONS *> cons_2;
  ret = ArcILPCreateConstraints2(scip, cons_2, f);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The link constraints is wrong.";
  else std::cout << "Constraints 2 created" << std::endl;

  std::vector<SCIP_CONS *> cons_3;
  ret = ArcILPCreateConstraints3(scip, cons_3, f, x);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The link constraints is wrong.";
  else std::cout << "Constraints 3 created" << std::endl;

  std::vector<SCIP_CONS *> cons_4;
  ret = ArcILPCreateConstraints4(scip, cons_4, x, y);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The link constraints is wrong.";
  else std::cout << "Constraints 4 created" << std::endl;

  std::vector<SCIP_CONS *> cons_5;
  ret = ArcILPCreateConstraints5(scip, cons_5, y, b);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The link constraints is wrong.";
  else std::cout << "Constraints 5 created" << std::endl;

  std::vector<SCIP_CONS *> cons_6;
  ret = ArcILPCreateConstraints6(scip, cons_6, f, b);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The link constraints is wrong.";
  else std::cout << "Constraints 6 created" << std::endl;

  std::vector<SCIP_CONS *> cons_9;
  ret = ArcILPCreateConstraints9(scip, cons_9, u, f);
  if (ret != SCIP_OKAY) LOG(ERROR) << "The link constraints is wrong.";
  else std::cout << "Constraints 9 created" << std::endl;

  SCIP_CALL((SCIPwriteOrigProblem(scip, "MLU_before.lp", nullptr, FALSE)));

  // Release the constraints
//  for (SCIP_CONS *con : cons_1) {
//    SCIP_CALL(SCIPreleaseCons(scip, &con));
//  }
  for (SCIP_CONS *con : cons_2) {
    SCIP_CALL(SCIPreleaseCons(scip, &con));
  }
  for (SCIP_CONS *con : cons_3) {
    SCIP_CALL(SCIPreleaseCons(scip, &con));
  }
  for (SCIP_CONS *con : cons_4) {
    SCIP_CALL(SCIPreleaseCons(scip, &con));
  }
  for (SCIP_CONS *con : cons_5) {
    SCIP_CALL(SCIPreleaseCons(scip, &con));
  }
  for (SCIP_CONS *con : cons_6) {
    SCIP_CALL(SCIPreleaseCons(scip, &con));
  }
  for (SCIP_CONS *con : cons_9) {
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

  // record the model
  SCIP_CALL((SCIPwriteOrigProblem(scip, "MLU.lp", nullptr, FALSE)));
  // Get the solutions
  SCIP_SOL *sol = nullptr;
  sol = SCIPgetBestSol(scip);

  res_f_ = std::vector<std::vector<std::vector<double>>>(numSbPerDcn,
                                                               std::vector<std::vector<double>>(
                                                                 numSbPerDcn));
  res_u_ = SCIPgetSolVal(scip, sol, u);
  std::cout << "problem result: " << res_u_ << std::endl;
  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb) {
    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
      if (src_sb == dst_sb) continue;
      std::cout << src_sb << "->" << dst_sb << ": ";
      for (int l = 0; l < links_.size(); ++l) {
        res_f_[src_sb][dst_sb].push_back(
          SCIPgetSolVal(scip, sol, x[src_sb][dst_sb][l]));
        std::cout << res_f_[src_sb][dst_sb][l] << ", ";
      }
      std::cout << std::endl;
    }
  }

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

void FullTopology::ArcILPResultAnalysis() {
  // traffic amount of each link
  std::vector<double> links_load = std::vector<double>(links_.size());
  std::cout << "The path with 0 traffic is not printed. " << std::endl;
  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb)
    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb)
      if (src_sb != dst_sb) {
        for (int p : per_sb_pair_paths_[src_sb][dst_sb]) {
          // print the traffic amount for each link
          double traffic_amount =
            traffic_matrix_[src_sb * numSbPerDcn + dst_sb] *
            scip_result_[src_sb][dst_sb][paths_[p].per_pair_id];
          if (traffic_amount > 0) {
            std::cout << traffic_amount << " Gbps of demand from u"
                      << src_sb << " -> u" << dst_sb
                      << " is placed on AbstractTopology link "
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
        for (int p : per_sb_pair_paths_[src_sb][dst_sb]) {
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