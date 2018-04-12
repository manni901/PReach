#ifndef CUT_H
#define CUT_H

#include "Util.h"

// A class modeling a vertex cut.
class Cut {
  Nodes middle;       // The nodes in the
  Nodes left;         // Set of nodes on the left
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
};

#endif