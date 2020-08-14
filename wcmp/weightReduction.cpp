//
// Created by wwt on 8/11/20.
//

#include "definitions.h"

namespace wcmp {

// get the group memory occupation
int GroupSize(const std::vector<int> &weights) {
  int total_size = 0;
  for (const int weight : weights) {
    total_size += weight;
  }
  return total_size;
}

// helper function for weight reduction
double CalcDelta(std::vector<int> &weights, std::vector<int> &new_weights) {
  double max_delta = 0;
  int total_size = GroupSize(weights);
  int new_total_size = GroupSize(new_weights);
  for (int i = 0; i < weights.size(); ++i) {
    double delta = new_weights[i] * total_size / (weights[i] * new_total_size);
    if (delta > max_delta) {
      max_delta = delta;
    }
  }
  return max_delta;
}

// helper function for weight reduction
int ChoosePortToUpdate(std::vector<int> &weights, std::vector<int> &new_weights) {
  double min_oversub = wcmp::infinity;
  int index = -1;
  int total_size = GroupSize(weights);
  int new_total_size = GroupSize(new_weights);
  for (int i = 0; i < weights.size(); ++i) {
    double oversub = (new_weights[i] + 1) * total_size /
                     double((new_total_size + 1) * weights[i]);
    if (oversub < min_oversub) {
      min_oversub = oversub;
      index = i;
    }
  }
  return index;
}

// the weight reduction function used in WCMP (eurosys)
std::vector<int> EurosysMethod(std::vector<int> weights) {
  std::vector<int> new_weights;
  for (const int weight : weights) {
    new_weights.emplace_back(weight);
  }
  int total_size = GroupSize(new_weights);
  while (total_size > maxGroupSize) {
    int non_reducible_size = 0;
    for (int i = 0; i < new_weights.size(); ++i) {
      if (new_weights[i] == 1) {
        non_reducible_size += new_weights[i];
      }
    }
    if (non_reducible_size == new_weights.size()) {
      break;
    }
    double reduction_ratio =
      (maxGroupSize - non_reducible_size) / double(GroupSize(weights));
    for (int i = 0; i < new_weights.size(); ++i) {
      new_weights[i] = int(weights[i] * reduction_ratio);
      if (new_weights[i] == 0) {
        new_weights[i] = 1;
      }
    }
    total_size = GroupSize(new_weights);
  }
  std::vector<int> results;
  for (const int weight : new_weights) {
    results.push_back(weight);
  }
  int remaining_size = maxGroupSize - GroupSize(new_weights);
  double min_oversub = CalcDelta(weights, new_weights);
  for (int k = 0; k < remaining_size; ++k) {
    int index = ChoosePortToUpdate(weights, new_weights);
    new_weights[index] += 1;
    if (min_oversub > CalcDelta(weights, new_weights)) {
      for (int i = 0; i < new_weights.size(); ++i) {
        results[i] = new_weights[i];
      }
      min_oversub = CalcDelta(weights, new_weights);
    }
  }
  return results;
}

} // namespace wcmp
