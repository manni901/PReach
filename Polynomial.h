#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include "Term.h"

class Polynomial {
public:
  Polynomial(Nodes source_nodes) {
    Term term;
    term.SetReachableNodes(source_nodes);
    terms_.push_back(term);
  }
  ~Polynomial() = default;

  void AddEdge(int edge_index, double p);

  void Collapse(Edges &mid_edges, Nodes &end_nodes, EDGE_INFO &edge_terminals);

  double GetResult();

private:
  vector<Term> terms_;
  unordered_map<string, Term> end_terms_;
};

#endif