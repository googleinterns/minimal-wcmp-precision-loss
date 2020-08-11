//
// Created by wwt on 6/14/20.
//

#ifndef WCMP_DEFINITIONS_H
#define WCMP_DEFINITIONS_H


#include <vector>
#include <string>
#include <iostream>
#include <ostream>
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <memory>
#include <limits>

namespace wcmp {

// the total amount of memory that can be used on one switch
const int tableSize = 16384;
// the maximum size of one WCMP group
const int maxGroupSize = 256;
// the intra domain link bandwidth
const double intraDomainBandwidth = 100;
// the virtual link bandwidth
const double maxBandwidth = 999999;
// the representation of infinity, used in SCIP
const double infinity = std::numeric_limits<double>::max();

// whether the solver is arc-based
const bool arc_based = true; // default: path-based
// whether the solver is integer LP
const bool integer_LP = false; // default: LP

enum class SwitchType {
  unknown = 0,
  s1 = 1, // s1 switch
  s2 = 2, // s2 switch
  s3 = 3, // s3 switch
  src = 4, // virtual source switch
  dst = 5, // virtual destination switch
};

enum class LinkType {
  unknown = 0,
  up = 1, // intra-domain up link
  down = 2, // intra-domain down link
  dcn = 3, // dcn link
  src = 4, // virtual source link
  dst = 5, // virtual destination link
};

struct Switch {
  const int superblock_id;
  const int middleblock_id;
  const int switch_id;
  const SwitchType switch_type;
  const int gid; // global id
};

struct Link {
  const int src_sw_gid;
  const int dst_sw_gid;
  const double capacity;
  const LinkType link_type;
  const int gid; // global id
};

struct Path {
  std::vector<int> link_gid_list;
  const int source_id;
  const int destination_id;
  const int per_pair_id;
  const int gid; // global id
};

class AbstractSolver;

} // namespace wcmp
#endif //WCMP_DEFINITIONS_H
