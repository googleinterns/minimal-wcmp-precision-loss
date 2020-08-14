//
// Created by wwt on 6/20/20.
//


#include <chrono>
#include <glog/logging.h>
#include "DCNOnlyTopology.h"

namespace wcmp {
namespace topo {
namespace dcnonly {

// the bandwidth of one SuperBlock for the DCN links
static double test_SBs[27] = {100, 200, 40, 40, 100, 200, 40, 100, 200,
                              40, 100, 200, 40, 100, 200, 40, 100, 200,
                              40, 100, 200, 40, 100, 200, 40, 100, 200};

// add all the switches to the network
void DCNOnlyTopology::AddSwitches() {
  int sw_gid = 0;
  // add s3 switches
  for (int i = 0; i < numSbPerDcn; ++i) {
    for (int j = 0; j < numMbPerSb; ++j) {
      for (int k = 0; k < numS3PerMb; ++k) {
        switches_.emplace_back(Switch({i, j, k, SwitchType::s3, sw_gid}));
        ++sw_gid;
      }
    }
  }
  // add virtual source switches
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    switches_.emplace_back(Switch({sb, -1, -1, SwitchType::src, sw_gid}));
    sources_.emplace_back(sw_gid);
    ++sw_gid;
  }
  // add virtual destination switches
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    switches_.emplace_back(Switch({sb, -1, -1, SwitchType::dst, sw_gid}));
    destinations_.emplace_back(sw_gid);
    ++sw_gid;
  }
}

// add all the links
void DCNOnlyTopology::AddLinks() {
  int link_gid = 0;
  per_pair_links_ = std::vector<std::vector<std::vector<int>>>(numSbPerDcn,
    std::vector<std::vector<int>>(numSbPerDcn));
  // determine source and destination superblocks
  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb) {
    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
      if (dst_sb != src_sb) {
        int links_per_sb = numLinkPerSb / (numSbPerDcn - 1);
        // determine source and destination middleblocks
        for (int src_mb = 0; src_mb < numMbPerSb; ++src_mb) {
          int dst_mb = src_mb; // only one color is allowed
          int links_per_mb = links_per_sb / numMbPerSb;
          // determine switch
          int cnt = 0;
          for (int i = 0; i < numS3PerMb*10; ++i) {
            // determine the source and destination s3
            int src = src_sb * numS3PerSb + src_mb * numS3PerMb + i % numS3PerMb;
            int dst = dst_sb * numS3PerSb + dst_mb * numS3PerMb + i % numS3PerMb;
            links_.emplace_back(Link({switches_[src].gid,
                                     switches_[dst].gid,
                                     std::min(test_SBs[src_sb], test_SBs[dst_sb]),
                                     LinkType::dcn,
                                     link_gid}));
            per_pair_links_[src_sb][dst_sb].emplace_back(link_gid);
            ++link_gid;
            ++cnt;
            if (cnt >= links_per_mb) break;
          }
        }
      }
    }
  }
  // add virtual source links
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    int src_gid = numS3PerSb*numSbPerDcn + sb;
    for (int i=0; i<numS3PerSb; ++i) {
      int sw_gid = numS3PerSb*sb+i;
      links_.emplace_back(Link({src_gid,
                               sw_gid,
                               maxBandwidth,
                               LinkType::src,
                               link_gid}));
      ++link_gid;
    }
  }
  // add virtual destination links
  for (int sb=0; sb<numSbPerDcn; ++sb) {
    int dst_gid = numS3PerSb*numSbPerDcn+numSbPerDcn+sb;
    for (int i=0; i<numS3PerSb; ++i) {
      int sw_gid = numS3PerSb*sb+i;
      links_.emplace_back(Link({sw_gid,
                               dst_gid,
                               maxBandwidth,
                               LinkType::dst,
                               link_gid}));
      ++link_gid;
    }
  }
}

// add all the paths
void DCNOnlyTopology::AddPaths() {
  int path_gid = 0;
  // initialize the dcn path vectors
  per_pair_paths_ = std::vector<std::vector<std::vector<int>>>(numSbPerDcn,
                                                               std::vector<std::vector<int>>(
                                                                 numSbPerDcn));

  // list and store all the paths
  for (int src_sb = 0; src_sb < numSbPerDcn; ++src_sb) {
    for (int dst_sb = 0; dst_sb < numSbPerDcn; ++dst_sb) {
      if (dst_sb != src_sb) {
        int index = 0;
        // add the direct paths
        for (const int link_gid : per_pair_links_[src_sb][dst_sb]) {

          paths_.emplace_back(Path({{link_gid},
                                     switches_[links_[link_gid].src_sw_gid].superblock_id,
                                     switches_[links_[link_gid].dst_sw_gid].superblock_id,
                                     index,
                                     path_gid})); // add path to the path list
          per_pair_paths_[src_sb][dst_sb].emplace_back(path_gid); // add path to per pair paths
          ++index;
          ++path_gid;
        }
        // add the transit paths
        for (int trans_sb = 0; trans_sb < numSbPerDcn; ++trans_sb) {
          if ((trans_sb != src_sb) && (trans_sb != dst_sb)) {
            for (int first_gid : per_pair_links_[src_sb][trans_sb]) {
              for (int second_gid : per_pair_links_[trans_sb][dst_sb]) {

                paths_.emplace_back(Path({
                                    {first_gid, second_gid},
                                    switches_[links_[first_gid].src_sw_gid].superblock_id,
                                    switches_[links_[second_gid].dst_sw_gid].superblock_id,
                                    index,
                                    path_gid}));
                per_pair_paths_[src_sb][dst_sb].emplace_back(path_gid);
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
DCNOnlyTopology::DCNOnlyTopology() {
  // add switches
  AddSwitches();
  // add links
  AddLinks();
  // add paths
  AddPaths();

  std::cout << "Topology initialized" << std::endl;
}

// TODO: can be optimized for clearance
void DCNOnlyTopology::PrintPath(const Path &path) {
  for (const int g : path.link_gid_list) {
    std::cout << links_[g].gid << " => ";
  }
  std::cout << "END" << std::endl;
}

} // namespace dcnonly
} // namespace topo
} // namespace wcmp