#ifndef RANDOM_SOLVER_H
#define RANDOM_SOLVER_H

#include "Term.h"
#include "Polynomial.h"
#include <algorithm>
#include <numeric>
#include <ctime>

class RandomSolver {
  Nodes source_nodes_;
  Nodes target_nodes_;
  Edges all_edges_;
  vector<int> shuffled_edges;
  Polynomial P_;

public:
  RandomSolver(Nodes &source_nodes, Nodes &target_nodes, Edges &all_edges)
      : source_nodes_(source_nodes), target_nodes_(target_nodes),
        all_edges_(all_edges), P_(Polynomial(source_nodes)) {
    int num_edges = all_edges_.count();
    shuffled_edges.resize(num_edges);
    iota(shuffled_edges.begin(), shuffled_edges.end(), 0);
    srand(time(0));
    random_shuffle(shuffled_edges.begin(), shuffled_edges.end());
  }
  
  double Solve(vector<EdgeInfo> &edge_info) {
      EDGE_INFO edge_terminals;
      for(int i = 0; i < int(edge_info.size()); i++) {
          edge_terminals[i] = edge_info[i].edge_terminals;
      }
      for(int edge_index : shuffled_edges) {          
          P_.AddEdge(edge_index, edge_info[edge_index].p);
          P_.Collapse(all_edges_, target_nodes_, edge_terminals);
      }
      return P_.GetResult();
  }

};

#endif