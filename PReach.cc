#include <fstream>
#include <lemon/bfs.h>
#include <lemon/list_graph.h>
#include <sstream>
#include "SausageSolver.h"
#include "RandomSolver.h"
#include "SamplingSolver.h"
#include <memory>
#include "Cut.h"
#include "Util.h"
#include "Graph.h"

using namespace std;

int main(int argc, char **argv) {
  if (argc < 5) {
    // arg1: network file
    // arg2: sources file
    // arg3: targets file
    // arg4: method (random, sausage, sampled)
    cout << "Usage: preach graph-file sources-file targets-file" << endl;
    return -1;
  }

  string file_name = argv[1];
  Graph graph(file_name);

  int numNodes = graph.CountNodes();
  int numEdges = graph.CountArcs();
  cout << endl << "Original graph size: " << numNodes << " nodes, " <<
  numEdges << " edges" << endl;

  // Read sources and targets and preprocess
  graph.Preprocess(argv[2], argv[3], PRE_YES);

  numNodes = graph.CountNodes();
  numEdges = graph.CountArcs();
  cout << endl << "Modified graph size: " << numNodes << " nodes, " <<
  numEdges << " edges" << endl << endl;

  if (numEdges == 0) { 
    // empty graph - source and target unreachable
    cout << "0.0" << endl;
    return 0;
  }

  unique_ptr<Solver> solver;
  string choice(argv[4]);

  double prob;

  if (choice == "random") {
      solver = make_unique<RandomSolver>(graph);
      prob = solver->Solve();
  } else if (choice == "sausage") {
      solver = make_unique<SausageSolver>(graph);
      prob = solver->Solve();
  } else {
      SamplingSolver sol(graph, 100, 0.9);
      prob = sol.Solve();
  }
  
  cout << "Reachability probability: " << prob;
  cout << endl;
  return 0;
}