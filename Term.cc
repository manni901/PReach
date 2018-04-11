#include "Term.h"

Term::Term() {
  coef_ = 1.0;
  X_.reset();
  Y_.set();
  reachable_nodes_.reset();
}

void Term::Multiply(int edge_index, double p, bool is_present) {
  if (is_present) {
    X_.set(edge_index);
    coef_ *= p;
  } else {
    Y_.reset(edge_index);
    coef_ *= (1 - p);
  }
}

bool Term::Collapse(Edges &mid_edges, Nodes &end_nodes,
                    EDGE_INFO &edge_terminals, Nodes &reachable_nodes) {
  // Lambda to compute all reachable node given visited nodes and
  // set of edges present.
  auto edge_visitor = [this, &edge_terminals](Edges &X) -> Nodes {
    vector<pair<int, int>> edges;
    for (int i = 0; i < NUM_EDGES; i++) {
      if (X[i]) {
        edges.push_back(edge_terminals[i]);
      }
    }
    int count = edges.size();
    Nodes visited = reachable_nodes_;
    // traverse the edges, setting targets as true until nothing changes.
    while (true) {
      Nodes copy = visited;
      for (int i = 0; i < count; i++) {
        if (visited[edges[i].first]) {
          visited.set(edges[i].second);
        }
      }
      if (copy == visited)
        break;
    }
    return visited;
  };

  // FIRST: traverse the x edges and see which end nodes are reachable
  // This is now the set of sure reachable nodes
  Nodes reachable = end_nodes & edge_visitor(X_);

  // SECOND: traverse all edges except y and see which end nodes are
  // unreachable
  Edges all_edges;
  all_edges.set();
  Edges yInverse = all_edges & Y_;

  // This is now the set of sure unreachable nodes
  Nodes unreachable = end_nodes & ~edge_visitor(yInverse);

  // LAST: if all end_nodes are either reachable or unreachable: collapsed
  // else (at least one node is neither reachable nor unreachable): doesn't
  // collapse.
  if (end_nodes == reachable) {
    reachable_nodes = reachable;
    return true;
  } else if (end_nodes == unreachable) {
    return true;
  } else {
    return false;
  }
}