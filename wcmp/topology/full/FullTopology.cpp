//
// Created by wwt on 6/20/20.
//


#include <chrono>
#include <glog/logging.h>
#include "FullTopology.h"

namespace wcmp {
namespace topo {
namespace full {

static double test_SBs[10] = {100, 100, 40, 40, 200, 100, 40, 100, 200, 200};

// add all the switches to the network
void FullTopology::AddSwitches() {
  int sw_gid = 0;
  per_sb_switches_ = std::vector<std::vector<int>>(numSbPerDcn);

  // add s3 switches
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    for (int mb=0; mb<numMbPerSb; ++mb) {
      for (int k = 0; k < numS3PerMb; ++k) {
        Switch tmp_sw = {sb, mb, k, SwitchType::s3, sw_gid};
        switches_.push_back(tmp_sw);
        per_sb_switches_[sb].push_back(sw_gid);
        ++sw_gid;
      }
    }
  }
  // add s2 switches
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    for (int mb=0; mb<numMbPerSb; ++mb) {
      for (int k = 0; k < numS2PerMb; ++k) {
        Switch tmp_sw = {sb, mb, k, SwitchType::s2, sw_gid};
        switches_.push_back(tmp_sw);
        per_sb_switches_[sb].push_back(sw_gid);
        ++sw_gid;
      }
    }
  }
  // add s1 switches
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    for (int mb=0; mb<numMbPerSb; ++mb) {
      for (int k = 0; k < numS1PerMb; ++k) {
        Switch tmp_sw = {sb, mb, k, SwitchType::s1, sw_gid};
        switches_.push_back(tmp_sw);
        per_sb_switches_[sb].push_back(sw_gid);
        ++sw_gid;
      }
    }
  }
  // add virtual S3 switch
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    Switch tmp_sw = {sb, -1, -1, SwitchType::vir, sw_gid};
    switches_.push_back(tmp_sw);
    s3_virtual_switches_.push_back(sw_gid);
    ++sw_gid;
  }
}

// add all the links
void FullTopology::AddLinks() {
  int link_gid = 0;
  per_sb_pair_links_ = std::vector<std::vector<std::vector<int>>>(numSbPerDcn,
    std::vector<std::vector<int>>(numSbPerDcn));
  per_s3_pair_links_ = std::vector<std::vector<std::vector<int>>>(numS3PerDCN,
    std::vector<std::vector<int>>(numS3PerDCN));
  per_switch_links_ = std::vector<std::vector<int>>(switches_.size());

  // add DCN links
  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb) {
    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
      if (src_sb == dst_sb) continue;
      // determine the number of links
      int links_per_sb;
      if (src_sb+1!=dst_sb) links_per_sb = floor(numLinkPerSb/(numSbPerDcn-1));
      else links_per_sb = floor(numLinkPerSb/(numSbPerDcn-1))+numLinkPerSb%(numSbPerDcn-1);
      // determine source and destination MiddleBlocks
      for (int src_mb = 0; src_mb < numMbPerSb; ++src_mb) {
        int dst_mb = src_mb; // only one color is allowed
        // determine the number of links
        int links_per_mb;
        if (src_mb != 0) links_per_mb = floor(links_per_sb/numMbPerSb);
        else links_per_mb = floor(links_per_sb/numMbPerSb) + links_per_sb%numMbPerSb;
        // determine switch pair
        int cnt = 0;
        for (int gap=0; gap<100; ++gap) {
          for (int src_sw = 0; src_sw < numS3PerMb; ++src_sw) {
            // determine the source and destination s3
            int src = src_sb * numS3PerSb + src_mb * numS3PerMb + src_sw;
            int dst = dst_sb * numS3PerSb + dst_mb * numS3PerMb + (src_sw+gap)%numS3PerMb;
            Link tmp_link = {switches_[src].gid,
                             switches_[dst].gid,
                             std::min(test_SBs[src_sb], test_SBs[dst_sb]),
                             LinkType::dcn,
                             link_gid};
            links_.push_back(tmp_link);
            per_sb_pair_links_[src_sb][dst_sb].push_back(link_gid);
            per_s3_pair_links_[src][dst].push_back(link_gid);
            per_switch_links_[src].push_back(link_gid);
            per_switch_links_[dst].push_back(link_gid);
            ++link_gid;
            ++cnt;
            if (cnt >= links_per_mb) break;
          }
          if (cnt >= links_per_mb) break;
        }
      }
    }
  }
  // add S2-S3 links
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    for (int mb=0; mb<numMbPerSb; ++mb) {
      for (int i=0; i<numS3PerMb; ++i) {
        int sw3 = sb*numSwPerSb+mb*numSwPerMb+i;
        for (int j=0; j<numS2PerMb; ++j) {
          int sw2 = sb*numSwPerSb+mb*numSwPerMb+numS3PerMb+j;
          // add bidirectional links
          Link tmp_link = {switches_[sw2].gid,
                           switches_[sw3].gid,
                           intraDomainBandwidth,
                           LinkType::up,
                           link_gid};
          links_.push_back(tmp_link);
          per_switch_links_[sw2].push_back(link_gid);
          per_switch_links_[sw3].push_back(link_gid);
          ++link_gid;
          Link tmp_link2 = {switches_[sw3].gid,
                           switches_[sw2].gid,
                           intraDomainBandwidth,
                           LinkType::down,
                           link_gid};
          links_.push_back(tmp_link2);
          per_switch_links_[sw2].push_back(link_gid);
          per_switch_links_[sw3].push_back(link_gid);
          ++link_gid;
        }
      }
    }
  }
  // add S1-S2 links
  // add virtual links at S3 level
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    for (int sw=0; sw<per_sb_switches_.size(); ++sw) {
      int sw_gid = per_sb_switches_[sb][sw];
      if (switches_[sw_gid].switch_type==SwitchType::s3) {
        Link tmp_link = {switches_[sw_gid].gid,
                         s3_virtual_switches_[sb],
                         maxBandwidth,
                         LinkType::vir,
                         link_gid};
        links_.push_back(tmp_link);
        per_switch_links_[sw_gid].push_back(link_gid);
        per_switch_links_[s3_virtual_switches_[sb]].push_back(link_gid);
        ++link_gid;
        Link tmp_link2 = {s3_virtual_switches_[sb],
                          switches_[sw_gid].gid,
                          maxBandwidth,
                          LinkType::vir,
                          link_gid};
        links_.push_back(tmp_link2);
        per_switch_links_[sw_gid].push_back(link_gid);
        per_switch_links_[s3_virtual_switches_[sb]].push_back(link_gid);
        ++link_gid;
      }
    }
  }
  // add virtual links at S2 level
}

// add all the paths
void FullTopology::AddPaths() {
  int path_gid = 0;
  // initialize the dcn path vectors
  per_pair_paths_ = std::vector<std::vector<std::vector<int>>>(numSbPerDcn,
                                                               std::vector<std::vector<int>>(
                                                                 numSbPerDcn));
  // list and store all the DCN paths
  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb)
    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb)
      if (dst_sb != src_sb) {
        int index = 0;
        // add the direct paths
        std::vector<int> direct_paths;
        direct_paths = this->FindLinks(src_sb, dst_sb);
        for (int link_gid : direct_paths) {
          Path tmp_path = {{link_gid},
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
                Path new_path = {
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
FullTopology::FullTopology() {
  // initial traffic matrix
  traffic::Trace trace;
  traffic_matrix_ = trace.GenerateTrafficMatrix(numSbPerDcn,
    traffic::TrafficPattern::kSparse);
  trace.ImportTrafficTrace("test");
  // add switches
  AddSwitches();
  // add DCN links
  AddLinks();
  // add DCN paths
  AddPaths();

  std::cout << "Topology initialized" << std::endl;
}

// return the links between two SuperBlocks
std::vector<int> FullTopology::FindLinks(int src_sb, int dst_sb) {
  std::vector<int> result;
  for (int link_gid : per_sb_pair_links_[src_sb][dst_sb]) {
    result.push_back(link_gid);
  }
  return result;
}

void FullTopology::PrintSwitch(int sw) {
  switch(switches_[sw].switch_type) {
    case SwitchType::s3:
      std::cout << "SB" << switches_[sw].superblock_id
                << " MB" << switches_[sw].middleblock_id
                << " S3-" << switches_[sw].gid;
      break;
    case SwitchType::s2:
      std::cout << "SB" << switches_[sw].superblock_id
                << " MB" << switches_[sw].middleblock_id
                << " S2-" << switches_[sw].gid;
      break;
    case SwitchType::s1:
      std::cout << "SB" << switches_[sw].superblock_id
                << " MB" << switches_[sw].middleblock_id
                << " S1-" << switches_[sw].gid;
      break;
    case SwitchType::vir:
      std::cout << "SB" << switches_[sw].superblock_id
//                << " MB" << switches_[sw].middleblock_id
                << " Virtual-" << switches_[sw].gid;
      break;
    case SwitchType::unknown:
      std::cout << "SB" << switches_[sw].superblock_id
                << " MB" << switches_[sw].middleblock_id
                << " Unknown-" << switches_[sw].gid;
      break;
  }
}

void FullTopology::PrintLink(int l) {
  switch(links_[l].link_type) {
    case LinkType::down:
      std::cout << "Down link " << links_[l].gid << ": ";
      break;
    case LinkType::up:
      std::cout << "Up link " << links_[l].gid << ": ";
      break;
    case LinkType::dcn:
      std::cout << "DCN link " << links_[l].gid << ": ";
      break;
    case LinkType::vir:
      std::cout << "Virtual link " << links_[l].gid << ": ";
      break;
    case LinkType::unknown:
      std::cout << "Unknown link " << links_[l].gid << ": ";
      break;
  }
  PrintSwitch(links_[l].src_sw_gid);
  std::cout << " -> ";
  PrintSwitch(links_[l].dst_sw_gid);
  std::cout << std::endl;
}

void FullTopology::PrintAllLinks() {
  for (int l=0; l<links_.size(); ++l) {
    PrintLink(l);
  }
}

// TODO: can be optimized for clearance
void FullTopology::PrintPath(const Path &path) {
  for (int g : path.link_gid_list) {
    std::cout << links_[g].gid << " => ";
  }
  std::cout << "END" << std::endl;
}

} // namespace full
} // namespace topo
} // namespace wcmp