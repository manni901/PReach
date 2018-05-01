#ifndef RANDOM_SOLVER_H
#define RANDOM_SOLVER_H

#include "Solver.h"
#include <algorithm>
#include <ctime>
#include <numeric>

class RandomSolver : public Solver {
  vector<int> shuffled_edges_;

public:
  RandomSolver(Graph &graph) : Solver(graph) {}

  double Solve() {
    Shuffle();
    EDGE_INFO edge_terminals;
    for (auto &info : edge_info_) {
      edge_terminals[info.first] = info.second.edge_terminals;
    }
    for (int edge_index : shuffled_edges_) {
      P_.AddEdge(edge_index, edge_info_[edge_index].p);
      P_.Collapse(all_edges_, target_, edge_terminals);
    }
    P_.Advance();
    return P_.GetResult();
  }

private:
  void Shuffle() {
    FOREACH_BS(edge_id, all_edges_) { shuffled_edges_.push_back(edge_id); }
    srand(time(0));
    random_shuffle(shuffled_edges_.begin(), shuffled_edges_.end());
  }
};

#endif