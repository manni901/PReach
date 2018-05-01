#ifndef CUT_H
#define CUT_H

#include "Util.h"
#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;

// A class modeling a vertex cut.
class Cut {
  Nodes left;         // Set of nodes on the left
  Nodes middle;       // The nodes in the
  Nodes right;        // Set of nodes on the right
  Edges coveredEdges; // Set of edges covered by this cut (left and middle)

public:
  Cut() {}

  // consturctor for a specified cut
  Cut(Nodes &_left, Nodes &_middle, Nodes &_right, Edges &_covered)
      : left(_left), middle(_middle), right(_right), coveredEdges(_covered) {}

  Nodes &getMiddle() { return middle; }
  Nodes &getRight() { return right; }
  Nodes &getLeft() { return left; }
  Edges &getCoveredEdges() { return coveredEdges; }
  int size() { return middle.count(); }

  bool Overlaps(Cut &right) {
    return (~middle & right.middle).none();
  }

  // Removes cuts that are obsoleted by cut
  // A cut is obsolete if its middle set overlaps with the left set of cut
  vector<Cut> RemoveObsoleteCuts(vector<Cut> &cuts) {
    vector<Cut> non_redundant_cuts;
    for(auto& currentCut : cuts) {
      // check if currentCut is non obsolete.
      if (!(currentCut.getMiddle() & getLeft()).any()) {
        non_redundant_cuts.emplace_back(move(currentCut));
      }
    }
    return non_redundant_cuts;
  }

  void Print(unordered_map<int, EdgeInfo> &edge_info) {
    Nodes nodes = getMiddle();
    FOREACH_BS(id, nodes) { cout << id << " "; }
    cout << " : ";
    Edges covered = getCoveredEdges();
    FOREACH_BS(id, covered) {
      cout << edge_info[id].edge_terminals.first << "-"
           << edge_info[id].edge_terminals.second << " ";
    }
    cout << endl;
  }
};

#endif