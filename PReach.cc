#include "Cut.h"
#include "Graph.h"
#include "RandomSolver.h"
#include "SamplingSolver.h"
#include "SausageSolver.h"
#include "Util.h"
#include <fstream>
#include <lemon/bfs.h>
#include <lemon/list_graph.h>
#include <memory>
#include <sstream>

using namespace std;

int main(int argc, char **argv) {
  if (argc < 5) {
    // arg1: network file
    // arg2: sources file
    // arg3: targets file
    // arg4: method (random, sausage, sampled)
    cout << "Usage: preach [network-file] [sources-file] [targets-file] "
            "[method] [success-prob] [num-iterations] [probe-size] [probe-repeat]"
         << endl;
    return -1;
  }

  string file_name = argv[1];
  Graph graph(file_name);

  int numNodes = graph.CountNodes();
  int numEdges = graph.CountArcs();
  cout << endl
       << "Original graph size: " << numNodes << " nodes, " << numEdges
       << " edges" << endl;

  // Read sources and targets and preprocess
  graph.Preprocess(argv[2], argv[3], PRE_YES);

  numNodes = graph.CountNodes();
  numEdges = graph.CountArcs();
  cout << endl
       << "Modified graph size: " << numNodes << " nodes, " << numEdges
       << " edges" << endl
       << endl;

  if (numEdges == 0) {
    // empty graph - source and target unreachable
    cout << "0.0" << endl;
    return 0;
  }

  unique_ptr<Solver> solver;
  double prob;

  string choice = argv[4];

  if (choice == "random") {
    solver = make_unique<RandomSolver>(graph);
    prob = solver->Solve();
  } else if (choice == "sausage") {
    solver = make_unique<SausageSolver>(graph);
    prob = solver->Solve();
  } else {
    double success_prob = atof(argv[5]);
    int num_iteration = atoi(argv[6]), probe_size = 0, probe_repeat = 0;
    bool fixed = false, weighted = false;
    if (choice != "sample-random") {
      fixed = true;
      probe_size = atoi(argv[7]);
      probe_repeat = atoi(argv[8]);
      weighted = choice == "sample-weighted";
    }
    SamplingSolver sol(graph, num_iteration, success_prob, probe_size, probe_repeat, fixed, weighted);
    prob = sol.Solve();
  }

  cout << "Reachability probability: " << prob;
  cout << endl;
  return 0;
}