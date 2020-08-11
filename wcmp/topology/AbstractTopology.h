//
// Created by wwt on 6/14/20.
//

#ifndef WCMP_ABSTRACTTOPOLOGY_H
#define WCMP_ABSTRACTTOPOLOGY_H


#include <iostream>
#include <vector>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include "definitions.h"
#include "trace/Trace.h"

namespace wcmp {
namespace topo {

// The virtual class for AbstractTopology network with a number of SuperBlocks,
// each of them has different northbound bandwidth.
class AbstractTopology {

protected:
  // all the switches instances
  std::vector<Switch> switches_;
  // all the links instances
  std::vector<Link> links_;
  // all the paths instances
  std::vector<Path> paths_;
  // all the paths for each source-destination pair
  std::vector<std::vector<std::vector<int>>> per_pair_paths_;
  // all the sources
  std::vector<int> sources_;
  // all the destinations
  std::vector<int> destinations_;

public:
  std::vector<Switch> GetSwitches() {
    return switches_;
  }
  std::vector<Link> GetLinks() {
    return links_;
  }
  std::vector<Path> GetPaths() {
    return paths_;
  }
  std::vector<std::vector<std::vector<int>>> GetPerPairPaths() {
    return per_pair_paths_;
  }
  std::vector<int> GetSources() {
    return sources_;
  }
  std::vector<int> GetDestinations() {
    return destinations_;
  }

};

} // namespace topo
} // namespace wcmp

#endif //WCMP_ABSTRACTTOPOLOGY_H
