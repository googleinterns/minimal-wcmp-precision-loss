//
// Created by wwt on 8/9/20.
//

#ifndef WCMP_ABSTRACTSOLVER_H
#define WCMP_ABSTRACTSOLVER_H

#include <iostream>
#include <vector>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include "definitions.h"
#include "trace/Trace.h"
#include "topology/AbstractTopology.h"

namespace wcmp {
namespace solver {

// The virtual class for solver allow flexible definition of different solvers
class AbstractSolver {

protected:
  std::unordered_map<int, double> traffic_matrix_;

  std::vector<Switch> switches_;
  std::vector<Link> links_;
  std::vector<Path> paths_;
  std::vector<std::vector<std::vector<int>>> per_pair_paths_;
  std::vector<int> sources_;
  std::vector<int> destinations_;

  std::vector<std::vector<int>> per_switch_links_;
  std::vector<std::vector<int>> per_link_paths_;

public:
  // construction function with given topology and traffic trace
  AbstractSolver(wcmp::topo::AbstractTopology topology, std::unordered_map<int, double> &trace) {
    switches_ = topology.GetSwitches();
    links_ = topology.GetLinks();
    paths_ = topology.GetPaths();
    per_pair_paths_ = topology.GetPerPairPaths();
    sources_ = topology.GetSources();
    destinations_ = topology.GetDestinations();
    traffic_matrix_ = trace;

    // get per switch links
    per_switch_links_ = std::vector<std::vector<int>>(switches_.size());
    for (int i=0; i<links_.size(); ++i) {
      per_switch_links_[links_[i].src_sw_gid].push_back(i);
      per_switch_links_[links_[i].dst_sw_gid].push_back(i);
    }
    // get per link paths
    per_link_paths_ = std::vector<std::vector<int>>(links_.size());
    for (int i=0; i<paths_.size(); ++i) {
      for (int l : paths_[i].link_gid_list) {
        per_link_paths_[l].push_back(i);
      }
    }
  }

  // find the best Routing strategy with the input
  virtual SCIP_RETCODE FindBestRouting() {};

};

} // namespace solver
} // namespace wcmp

#endif //WCMP_ABSTRACTSOLVER_H
