#ifndef TERM_H
#define TERM_H

#include <bitset>

#define NUM_EDGES 1024

typedef std::bitset<NUM_EDGES> Edges;

// Term respresents a particular subset of the graph
// with its probability coefficient.
class Term {
private:
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
  void Multiply(int edge_index, double p, bool isPresent);
};

#endif