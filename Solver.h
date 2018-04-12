#ifndef SOLVER_H
#define SOLVER_H

#include "Polynomial.h"

class Solver {
public:
  Solver(Nodes source, Nodes target, unordered_map<int, EdgeInfo>& edge_info)
      : source_(source), target_(target), edge_info_(move(edge_info)), P_(Polynomial(source)) {}

  virtual double Solve() = 0;

  void SetAllEdges(Edges all_edges) { all_edges_ = all_edges; }

protected:
  Nodes source_;
  Nodes target_;
  unordered_map<int, EdgeInfo> edge_info_;
  Polynomial P_;
  Edges all_edges_;
};

#endif