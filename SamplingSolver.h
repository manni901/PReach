#ifndef SAMPLING_SOLVER_H
#define SAMPLING_SOLVER_H

#include "Graph.h"
#include "SausageSolver.h"
#include "Util.h"
#include <random>
#include <sys/time.h>
using namespace std;
mt19937 theRandomMT;
uniform_real_distribution<double> theRandomGenerator;

class SamplingSolver {
public:
  SamplingSolver(Graph &graph, int num_iteration, double success_prob)
      : num_iteration_(num_iteration), success_prob_(success_prob),
        graph_(graph) {
    initRand();
  }
  double Solve() {
    double result = 0.0;
    for (int i = 0; i < num_iteration_; i++) {
      Graph graph;
      graph.CopyFrom(graph_);

      graph.Print();

      auto prob_map = SampleRandom();

      graph.UpdateWeights(prob_map);
      graph.Minimize();

      graph.Print();
      if (graph.CountArcs() > 1) {
        SausageSolver solver(graph);
        result += solver.Solve();
      } else if (graph.CountArcs() == 1)
        result += 1.0;
      else
        result += 0.0;
    }
    return result / double(num_iteration_);
  }

private:
  int num_iteration_;
  double success_prob_;
  Graph &graph_;

  void initRand() {
    timeval time;
    gettimeofday(&time, NULL);
    theRandomMT.seed((time.tv_sec * 1000) + (time.tv_usec / 1000));
    theRandomGenerator = std::uniform_real_distribution<double>(0.0, 1.0);
  }

  double nextRand() { return theRandomGenerator(theRandomMT); }

  // Returns a map keyed by edge id and values giving the random
  // probability for that. Edges with probability greater than
  // their weight can then be replaced by 1 and less than by 0.
  unordered_map<int, double> SampleFixed(Edges &sampleEdges) {
    unordered_map<int, double> edge_prob;
    FOREACH_BS(edge_id, sampleEdges) { edge_prob[edge_id] = nextRand(); }
    return edge_prob;
  }

  unordered_map<int, double> SampleRandom() {
    unordered_map<int, double> edge_prob;
    Edges all_edges = graph_.EdgesAsBitset();
    FOREACH_BS(edge_id, all_edges) {
      if (nextRand() >= success_prob_) {
        edge_prob[edge_id] = nextRand();
      }
    }
    return edge_prob;
  }
};

#endif