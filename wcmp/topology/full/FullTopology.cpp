//
// Created by wwt on 6/20/20.
//


#include <chrono>
#include <glog/logging.h>
#include "FullTopology.h"

namespace wcmp {
namespace topo {
namespace full {

// the bandwidth of one SuperBlock for the DCN links
static double test_SBs[27] = {40, 100, 200, 40, 100, 200, 40, 100, 200,
                              40, 100, 200, 40, 100, 200, 40, 100, 200,
                              40, 100, 200, 40, 100, 200, 40, 100, 200};

// add all the switches to the network
void FullTopology::AddSwitches() {
  int sw_gid = 0;
  // add s3 switches
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    for (int mb=0; mb<numMbPerSb; ++mb) {
      for (int k = 0; k < numS3PerMb; ++k) {
        Switch tmp_sw = {sb, mb, k, SwitchType::s3, sw_gid};
        switches_.emplace_back(tmp_sw);
        ++sw_gid;
      }
    }
  }
  // add s2 switches
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    for (int mb=0; mb<numMbPerSb; ++mb) {
      for (int k = 0; k < numS2PerMb; ++k) {
        Switch tmp_sw = {sb, mb, k, SwitchType::s2, sw_gid};
        switches_.emplace_back(tmp_sw);
        ++sw_gid;
      }
    }
  }
  // add virtual source switch
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    Switch tmp_sw = {sb, -1, -1, SwitchType::src, sw_gid};
    switches_.emplace_back(tmp_sw);
    ++sw_gid;
  }
  // add virtual destination switch
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    Switch tmp_sw = {sb, -1, -1, SwitchType::dst, sw_gid};
    switches_.emplace_back(tmp_sw);
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

  // add AbstractTopology links
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
            links_.emplace_back(tmp_link);
            per_sb_pair_links_[src_sb][dst_sb].emplace_back(link_gid);
            per_s3_pair_links_[src][dst].emplace_back(link_gid);
            per_switch_links_[src].emplace_back(link_gid);
            per_switch_links_[dst].emplace_back(link_gid);
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
        int sw3 = sb*numS3PerSb+mb*numS3PerMb+i;
        for (int j=0; j<numS2PerMb; ++j) {
          int sw2 = numS3PerDCN+sb*numS2PerSb+mb*numS2PerMb+numS3PerMb+j;
          // add bidirectional links
          Link tmp_link = {switches_[sw2].gid,
                           switches_[sw3].gid,
                           intraDomainBandwidth,
                           LinkType::up,
                           link_gid};
          links_.emplace_back(tmp_link);
          per_switch_links_[sw2].emplace_back(link_gid);
          per_switch_links_[sw3].emplace_back(link_gid);
          ++link_gid;
          Link tmp_link2 = {switches_[sw3].gid,
                           switches_[sw2].gid,
                           intraDomainBandwidth,
                           LinkType::down,
                           link_gid};
          links_.emplace_back(tmp_link2);
          per_switch_links_[sw2].emplace_back(link_gid);
          per_switch_links_[sw3].emplace_back(link_gid);
          ++link_gid;
        }
      }
    }
  }
  // add virtual source links
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    int src_gid = numS3PerDCN + numS2PerDCN + sb;
    for (int i=0; i<numS2PerSb; ++i) {
      int sw_gid = numS3PerDCN+numS2PerSb*sb+i;
      Link tmp_link = {src_gid,
                       sw_gid,
                       maxBandwidth,
                       LinkType::src,
                       link_gid};
      links_.emplace_back(tmp_link);
      ++link_gid;
    }
  }
  // add virtual destination links
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    int dst_gid = numS3PerDCN+numS2PerDCN+numSbPerDcn+sb;
    for (int i=0; i<numS2PerSb; ++i) {
      int sw_gid = numS3PerDCN+numS2PerSb*sb+i;
      Link tmp_link = {sw_gid,
                       dst_gid,
                       maxBandwidth,
                       LinkType::dst,
                       link_gid};
      links_.emplace_back(tmp_link);
      ++link_gid;
    }
  }
}

std::vector<Path> FullTopology::FindDCNPaths(int src_sw, int dst_sw) {
  std::vector<Path> paths;
  // add the direct paths
  std::vector<int> direct_paths;
  direct_paths = this->FindLinks(src_sw, dst_sw);
  for (int link_gid : direct_paths) {
    Path tmp_path = {{link_gid},
                     links_[link_gid].src_sw_gid,
                     links_[link_gid].dst_sw_gid,
                     -1, -1};
    paths.emplace_back(tmp_path); // add path to the path list
  }
}

// add all the paths
void FullTopology::AddPaths() {
  int path_gid = 0;
  // list and store all the paths
  for (int src_idx = 0; src_idx < sources_.size(); ++src_idx) {
    for (int dst_idx = 0; dst_idx < destinations_.size(); ++dst_idx) {
      if (sources_[src_idx] != destinations_[dst_idx]) {
        int index = 0;
        // add the direct paths
        std::vector<int> direct_paths;
        direct_paths = this->FindLinks(src_idx, dst_idx);
        for (int link_gid : direct_paths) {
          Path tmp_path = {{link_gid},
                           links_[link_gid].src_sw_gid,
                           links_[link_gid].dst_sw_gid,
                           index, path_gid};
          paths_.emplace_back(tmp_path); // add path to the path list
          per_sb_pair_paths_[src_idx][dst_idx].emplace_back(
            path_gid); // add path to per pair paths
          per_link_paths_[link_gid].emplace_back(path_gid);
          ++index;
          ++path_gid;
        }
        // add the transit paths
        for (int trans_sb = 0; trans_sb < numSbPerDcn; ++trans_sb) {
          if ((trans_sb != src_idx) && (trans_sb != dst_idx)) {
            std::vector<int> first_hops;
            std::vector<int> second_hops;
            first_hops = this->FindLinks(src_idx, trans_sb);
            second_hops = this->FindLinks(trans_sb, dst_idx);
            for (int first_gid : first_hops) {
              for (int second_gid : second_hops) {
                Path new_path = {
                  {first_gid, second_gid},
                  links_[first_gid].src_sw_gid,
                  links_[second_gid].dst_sw_gid, index,
                  path_gid};
                paths_.emplace_back(new_path);
                per_sb_pair_paths_[src_idx][dst_idx].emplace_back(
                  path_gid);
                per_link_paths_[first_gid].emplace_back(path_gid);
                per_link_paths_[second_gid].emplace_back(path_gid);
                ++index;
                ++path_gid;
              }
            }
          }
        }
      }
    }
  }
}

// constructor function
FullTopology::FullTopology() {
  // add switches
  AddSwitches();
  // add links
  AddLinks();
  // add paths
  AddPaths();

  std::cout << "Topology initialized" << std::endl;
}

// return the links between two SuperBlocks
std::vector<int> FullTopology::FindLinks(int src_sb, int dst_sb) {
  std::vector<int> result;
  for (int link_gid : per_sb_pair_links_[src_sb][dst_sb]) {
    result.emplace_back(link_gid);
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
    case SwitchType::src:
      std::cout << "SB" << switches_[sw].superblock_id
                << " Virtual-" << switches_[sw].gid;
      break;
    case SwitchType::dst:
      std::cout << "SB" << switches_[sw].superblock_id
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
      std::cout << "AbstractTopology link " << links_[l].gid << ": ";
      break;
    case LinkType::src:
      std::cout << "Virtual link " << links_[l].gid << ": ";
      break;
    case LinkType::dst:
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