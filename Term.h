#ifndef TERM_H
#define TERM_H

#include <bitset>
#include <iostream>
#include <unordered_map>
#include <vector>

#define NUM_EDGES 1024
#define NUM_NODES 512

using namespace std;

typedef bitset<NUM_EDGES> Edges;
typedef bitset<NUM_NODES> Nodes;
typedef unordered_map<int, pair<int, int>> EDGE_INFO;

// Term respresents a particular subset of the graph
// with its probability coefficient.
class Term {
private:
  Nodes reachable_nodes_;
  // Bit vector for edges that are present in the term.
  Edges X_;

  // Bit vector for all edges except the ones which are
  // for sure absent in this term. By default all the edges
  // are set for Y_.
  Edges Y_;

  // Coefficient which denotes the probability of this subset.
  double coef_;

public:
  Term();

  // Adds a new edge term to the existing one.
  // The coefficient is updated with p or (1-p) depending on the
  // value of isPresent.
  void Multiply(int edge_index, double p, bool is_present);

  // Return true if all end_nodes are reachable
  bool Collapse(Nodes end_nodes, EDGE_INFO &edge_terminals);
};

#endif