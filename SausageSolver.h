#ifndef SAUSAGE_SOLVER_H
#define SAUSAGE_SOLVER_H

#include "Solver.h"
#include "Cut.h"
#include "Graph.h"

class SausageSolver : public Solver {
public:
  SausageSolver(Graph &graph) : Solver(graph) {
    cuts_ = graph.FindSomeGoodCuts();
    cout << "Bench 1\n";
  }

  double Solve() {
  
    Edges covered;
    Edges sausage;

    // repeat until no cuts left
    while (cuts_.size() > 0) {
      // select a cut: here we just select the first one (arbitrary)
      Cut nextCut = cuts_.front();
      // cout << "Available " << cuts.size() << " cuts, Using cut with size " <<
      // nextCut.size();
      // cout << nextCut.size() << "  ";
      cuts_.erase(cuts_.begin());
      // Identify the sausage: The current set of edges in question
      sausage = nextCut.getCoveredEdges() & ~covered;
      // cout << ", Sausage size: " << sausage.count() << endl;
      //cout << sausage.count() << "  ";
      // Consume the current sausage
      ConsumeSausage(sausage, nextCut.getMiddle());
      // mark the sausage as covered
      covered |= sausage;
      // remove obsolete cuts
      cuts_ = nextCut.RemoveObsoleteCuts(cuts_);
    }

    sausage = all_edges_ & ~covered;
    ConsumeSausage(sausage, target_);

    // RESULT
    return P_.GetResult();
    // return -1.0;
  }

  void ConsumeSausage(Edges& sausage, Nodes& end_nodes) {
    // Build a dictionary of edgeId -> source and target node ids
    // Will need it with each collapsation operation within this sausage
    unordered_map<int, pair<int, int>> edgeTerminals;
    FOREACH_BS(edgeId, sausage) {
      edgeTerminals[edgeId] = edge_info_[edgeId].edge_terminals;
    }

    // start adding the edges in the current sausage
    // here we collapse after each addition (arbitrary)
    int edgeCounter = 0;
    FOREACH_BS(edgeId, sausage) {
      edgeCounter++;
      // cout << "Adding edge " << edgeCounter;
      P_.AddEdge(edgeId, edge_info_[edgeId].p);
      // cout << ", Collapsing!" << endl;
      P_.Collapse(sausage, end_nodes, edgeTerminals);
    }

    // Advance the polynomial: make it ready for next sausage
    P_.Advance();
  }

protected:
  vector<Cut> cuts_;
};

#endif