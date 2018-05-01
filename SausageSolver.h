#ifndef SAUSAGE_SOLVER_H
#define SAUSAGE_SOLVER_H

#include "Cut.h"
#include "Graph.h"
#include "Solver.h"

class SausageSolver : public Solver {
public:
  SausageSolver(Graph &graph) : Solver(graph) {
    cuts_ = graph.FindSomeGoodCuts();
  }

  double Solve();

protected:
  vector<Cut> cuts_;

private:
  void ConsumeSausage(Edges &sausage, Nodes &end_nodes);
};

#endif