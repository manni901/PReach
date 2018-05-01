#ifndef EDGE_SUBSET_H
#define EDGE_SUBSET_H

#include "Cut.h"
#include "Util.h"
#include <unordered_map>

// EdgeSubset is used for weighted sampling purposes.
// Each EdgeSubset describes a set of edges for left nodes to
// middle nodes of a cut. A weight is assigned to each EdgeSubset.
// Higher weight means the chances of getting picked for sampling
// is higher.
class EdgeSubset {
public:
  EdgeSubset() = default;
  EdgeSubset(int id, Cut &cut, unordered_map<int, EdgeInfo> &edge_info) : id_(id) {
    Nodes &left = cut.getLeft();
    Nodes &middle = cut.getMiddle();
    for (auto &info : edge_info) {
      if (left.test(info.second.edge_terminals.first) &&
          middle.test(info.second.edge_terminals.second)) {
        edges_.set(info.first);
        success_prob_ *= (1.0 - info.second.p);
      }
    }
    success_prob_ += 0.001;
    cut_size_ = middle.count();
  }

  double Weight() { return success_prob_ * cut_size_ / edges_.count(); }

  int Id() {return id_;}

  Edges& GetEdges() { return edges_; }

  int CutSize() { return cut_size_;}

private:
  int id_;
  Edges edges_;
  int cut_size_;
  double success_prob_ = 1.0;
};

#endif