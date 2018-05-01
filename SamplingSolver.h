#ifndef SAMPLING_SOLVER_H
#define SAMPLING_SOLVER_H

#include "EdgeSubset.h"
#include "Graph.h"
#include "SausageSolver.h"
#include "Util.h"
#include <limits>
#include <random>
#include <sys/time.h>
using namespace std;

class SamplingSolver {
public:
  SamplingSolver(Graph &graph, int num_iteration, double success_prob,
                 int probe_size, int probe_repeat, bool fixed, bool weighted)
      : num_iteration_(num_iteration), success_prob_(success_prob),
        probe_size_(probe_size), probe_repeat_(probe_repeat), fixed_(fixed),
        weighted_(weighted), graph_(graph) {
    InitRand();
  }

  // Main solver method. It decides what kind of sampling to use and
  // then performs the calculation a number of times and averages the result.
  double Solve();

private:
  // Total number of iterations to perform.
  int num_iteration_;

  // Probability of bernoulli sampling.
  double success_prob_;

  // When probing for best sample, the number of times to probe.
  int probe_size_;

  // For each probe sample, number of times to perform calculation
  // for averaging.
  int probe_repeat_;

  // Flag to decide if sampling is fixed or random everytime.
  bool fixed_;

  // Flag to decide whether to perform weighted sampling by cut size
  // when fixed_ is set to true.
  bool weighted_;

  // Reference to main graph passed in my main.
  Graph &graph_;

  mt19937 theRandomMT_;

  uniform_real_distribution<double> theRandomGenerator_;

  // Initialize seed for random number generation.
  void InitRand();

  // Get a random number between 0 and 1.
  double NextRand() { return theRandomGenerator_(theRandomMT_); }

  // Get current time for time measurement.
  double GetCPUTime() { return (double)clock() / (CLOCKS_PER_SEC / 1000); }

  // Returns a map keyed by edge id and values giving the random
  // probability for that edge. Edges with probability greater than
  // their weight can then be replaced by 1 and less than by 0.
  // sampleEdges denotes the fixed edges found from probing. Only
  // these edges will be sampled.
  unordered_map<int, double> SampleFixed(Edges &sampleEdges);

  // In contrast to SampleFixed, this method samples all the edges
  // in the graph with success_prob_ probability. 
  unordered_map<int, double> SampleRandom();

  // Same as above but samples edges weighted by the cut size of the
  // cut in which those edges occur.
  // As input it takes in a vector of edge_subsets.
  unordered_map<int, double>
  SampleWeightedRandom(vector<EdgeSubset> &edge_subsets);

  // Finds out the set of fixed edges to pass to SampleFixed.
  // Probing means trying out differents combinations of edges to
  // sample and choosing the one which takes minimum time.
  Edges Probe();

  // Creates a weighted vector of edge_subset indices depending on
  // weight.
  vector<int> GetChanceVector(vector<EdgeSubset> &edge_subsets);
};

#endif