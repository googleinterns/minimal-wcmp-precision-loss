//
// Created by wwt on 6/20/20.
//


#include <chrono>
#include <glog/logging.h>
#include "IntraDomainTopology.h"

namespace wcmp {
namespace topo {
namespace intradomain {

// the bandwidth of one SuperBlock for the DCN links
static double test_SBs[27] = {100, 200, 40, 40, 100, 200, 40, 100, 200,
                              40, 100, 200, 40, 100, 200, 40, 100, 200,
                              40, 100, 200, 40, 100, 200, 40, 100, 200};

// add all the switches to the network
void IntraDomainTopology::AddSwitches() {
  int sw_gid = 0;
  // add s3 switches
  for (int k = 0; k < numS3PerMb; ++k) {
    switches_.emplace_back(Switch({0, 0, k, SwitchType::s3, sw_gid}));
    ++sw_gid;
  }
  // add s2 switches
  for (int k = 0; k < numS2PerMb; ++k) {
    switches_.emplace_back(Switch({0, 0, k, SwitchType::s2, sw_gid}));
    ++sw_gid;
  }
  // add s1 switches
  for (int k = 0; k < numS1PerMb; ++k) {
    switches_.emplace_back(Switch({0, 0, k, SwitchType::s1, sw_gid}));
    ++sw_gid;
  }
  // add virtual source switches
  for (int i=0; i<numS1PerMb; ++i) {
    switches_.emplace_back(Switch({0, 0, i, SwitchType::src, sw_gid}));
    sources_.emplace_back(sw_gid);
    ++sw_gid;
  }
  // add virtual destination switches
  for (int i=0; i<numPrefix; ++i) {

    switches_.emplace_back(Switch({0, 0, i, SwitchType::dst, sw_gid}));
    destinations_.emplace_back(sw_gid);
    ++sw_gid;
  }
}

// add all the links
void IntraDomainTopology::AddLinks() {
  int link_gid = 0;
  // add links between s2 and s3
  for (int s2=0; s2<numS2PerMb; ++s2) {
    for (int s3=0; s3<numS3PerMb; ++s3) {
      int s2_gid = numS3PerMb+s2;
      int s3_gid = s3;

      links_.emplace_back(Link({s2_gid,
                               s3_gid,
                               S2S3Bandwidth,
                               LinkType::up,
                               link_gid}));
      ++link_gid;
    }
  }
  // add links between s1 and s2
  for (int s1=0; s1<numS1PerMb; ++s1) {
    for (int i=0; i<2; ++i) {
      int s2 = (s1+i)%numS2PerMb;
      int s1_gid = numS3PerMb+numS2PerMb+s1;
      int s2_gid = numS3PerMb+s2;
      links_.emplace_back(Link({s1_gid,
                               s2_gid,
                               S1S2Bandwidth,
                               LinkType::up,
                               link_gid}));
      ++link_gid;
    }
  }

  // add virtual source links
  for (int i=0; i<numS1PerMb; ++i) {
    int src_gid = numS3PerMb+numS2PerMb+numS1PerMb + i;
    int sw_gid = numS3PerMb+numS2PerMb+i;
    links_.emplace_back(Link({src_gid,
                             sw_gid,
                             maxBandwidth,
                             LinkType::src,
                             link_gid}));
    ++link_gid;
  }
  // add virtual destination links
  for (int i=0; i<numPrefix; ++i) {
    int dst_gid = numS3PerMb+numS2PerMb+numS1PerMb+numS1PerMb+i;
    for (int j=0; j<numS3PerMb; ++j) {
      int sw_gid = j;
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
void IntraDomainTopology::AddPaths() {
  int path_gid = 0;
  // no path added
  per_pair_paths_ = std::vector<std::vector<std::vector<int>>>(1,
    std::vector<std::vector<int>>(1));
}

// constructor function
IntraDomainTopology::IntraDomainTopology() {
  // add switches
  AddSwitches();
  // add links
  AddLinks();
  // add paths
  AddPaths();

  std::cout << "Topology initialized" << std::endl;
}

void IntraDomainTopology::PrintSwitch(int sw) {
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

void IntraDomainTopology::PrintLink(int l) {
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

void IntraDomainTopology::PrintAllLinks() {
  for (int l=0; l<links_.size(); ++l) {
    PrintLink(l);
  }
}

void IntraDomainTopology::PrintPath(const Path &path) {
  for (const int g : path.link_gid_list) {
    std::cout << links_[g].gid << " => ";
  }
  std::cout << "END" << std::endl;
}

} // namespace dcnonly
} // namespace topo
} // namespace wcmp