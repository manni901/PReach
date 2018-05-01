#ifndef SOLVER_H
#define SOLVER_H

#include "Graph.h"
#include "Polynomial.h"

class Solver {
public:
  Solver(Graph &graph)
      : source_(graph.GetNodeBitset(SOURCE)), P_(Polynomial(source_)) {
    target_ = graph.GetNodeBitset(SINK);
    graph.GetEdgeInfo(edge_info_);
    all_edges_ = graph.EdgesAsBitset();
  }

  virtual double Solve() = 0;

protected:
  Nodes source_;
  Nodes target_;
  unordered_map<int, EdgeInfo> edge_info_;
  Polynomial P_;
  Edges all_edges_;
};

#endif