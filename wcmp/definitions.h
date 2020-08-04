//
// Created by wwt on 6/14/20.
//

#ifndef WCMP_DEFINITIONS_H
#define WCMP_MAIN_H


#include <vector>
#include <string>
#include <iostream>
#include <ostream>
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <memory>

namespace wcmp {

const int tableSize = 16384;
const int maxGroupSize = 256;
const double intraDomainBandwidth = 100;
const double maxBandwidth = 999999;

const bool arc_based = false; // default: path-based
const bool integer_LP = false; // default: LP
const int problem_scope = 0; // 0: S3-S3, 1: S2-S2, 2: S1-S3 (intra-domain) 3: S1-S2 (intra-domain)

enum class SwitchType {
  unknown = 0,
  s1 = 1,
  s2 = 2,
  s3 = 3,
  vir = 4,
};

enum class LinkType {
  unknown = 0,
  up = 1,
  down = 2,
  dcn = 3,
  vir = 4,
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
  const int src_sw_gid;
  const int dst_sw_gid;
  const int per_pair_id;
  const int gid; // global id
};

} // namespace wcmp
#endif //WCMP_DEFINITIONS_H
