#include "Graph.h"
#include <unordered_set>

void Graph::AddEdge(string source, string target, double weight) {
  if (edges_.insert(source + target).second) {
    ListDigraph::Arc arc = g_.addArc(GetNode(source), GetNode(target));
    weights_[arc] = weight;
  }
}

void Graph::UpdateWeights(unordered_map<int, double> edge_weights) {
  for (ListDigraph::ArcIt arc(g_); arc != INVALID; ++arc) {
    string node_source = nodes_[g_.source(arc)];
    string node_target = nodes_[g_.target(arc)];
    if ( node_source == SOURCE || node_target == SINK) continue;
    int edge_id = g_.id(arc);
    auto prob = edge_weights.find(edge_id);
    if(prob != edge_weights.end()) {
      if(prob->second > weights_[arc]) weights_[arc] = 1.0;
      else {
        weights_[arc] = 0.0;
        g_.erase(arc);
      }
    }
  }
}

void Graph::Reverse() {
  // Collect a list of all edges
  vector<ListDigraph::Arc> arcs;
  for (ListDigraph::ArcIt arc(g_); arc != INVALID; ++arc) {
    arcs.push_back(arc);
  }
  for (auto &arc : arcs) {
    g_.reverseArc(arc);
  }
}

ListDigraph::Node Graph::GetNode(string name) {
  auto node = name_to_node_.find(name);
  if (node != name_to_node_.end()) {
    return node->second;
  }
  ListDigraph::Node new_node = g_.addNode();
  nodes_[new_node] = name;
  name_to_node_[name] = new_node;
  return new_node;
}

void Graph::ReadList(string file_name, vector<string> &list) {
  ifstream in(file_name);
  string item;
  while (in >> item) {
    if (!item.empty())
      list.push_back(item);
  }
  in.close();
}

void Graph::Create(string &file_name) {
  ifstream in(file_name);
  while (!in.eof()) {
    string start;
    string stop;
    double weight = -1.0;
    in >> start >> stop >> weight;

    if (weight == -1.0)
      continue;

    AddEdge(start, stop, weight);
  }
  in.close();
}

void Graph::Minimize() {

    RemoveIsolatedNodes();
    CollapseELementaryPaths();
    RemoveSelfCycles();
}

void Graph::Preprocess(string sources_file, string targets_file, string pre) {
  UnifyTerminals(sources_file, targets_file);
  if (pre == PRE_YES) {
    Minimize();
  }
  // EXTRA STEP: make sure source and sink are not directly connected
  ListDigraph::Node source = name_to_node_[SOURCE];
  ListDigraph::Node sink = name_to_node_[SINK];
  for (ListDigraph::OutArcIt arc(g_, source); arc != INVALID; ++arc) {
    if (g_.id(g_.target(arc)) == g_.id(sink)) { // direct source-sink connection
                                                // - isolate with a middle node
      ListDigraph::Node isolator = g_.addNode();
      name_to_node_["ISOLATOR"] = isolator;
      nodes_[isolator] = "ISOLATOR";
      ListDigraph::Arc head = g_.addArc(isolator, sink);
      weights_[head] = weights_[arc];
      ListDigraph::Arc tail = g_.addArc(source, isolator);
      weights_[tail] = 1.0;
      g_.erase(arc);
      break;
    }
  }
}

void Graph::RemoveIsolatedNodes() {
  auto set_reachable_nodes = [this](string source, Nodes &node_set) {
    Bfs<ListDigraph> bfs(g_);
    bfs.run(name_to_node_[source]);
    for (ListDigraph::NodeIt node(g_); node != INVALID; ++node) {
      if (bfs.reached(node)) {
        node_set.set(g_.id(node));
      }
    }
  };
  // First: Make a forward traversal and mark the reachable nodes from source
  Nodes forward;
  set_reachable_nodes(SOURCE, forward);

  // Second: reverse the graph and make a backward traversal
  // and mark the reachable nodes from the sink
  Nodes backward;
  Reverse();
  set_reachable_nodes(SINK, backward);

  // reverse the graph again to return it to original state
  Reverse();

  // collect bad nodes
  for (auto &node : name_to_node_) {
    if (node.first == SOURCE || node.first == SINK)
      continue;
    if (!(forward[g_.id(node.second)] && backward[g_.id(node.second)])) {
      g_.erase(node.second);
      name_to_node_.erase(node.first);
    }
  }
}

void Graph::RemoveSelfCycles() {
  for (ListDigraph::ArcIt arc(g_); arc != INVALID; ++arc) {
    if (g_.source(arc) == g_.target(arc)) {
      g_.erase(arc);
    }
  }
}

void Graph::CollapseELementaryPaths() {
  // repeat until nothing changes
  bool changing = true;
  while (changing) {
    changing = false;
    RemoveSelfCycles();
    vector<ListDigraph::Node> elementaryNodes;
    for (ListDigraph::NodeIt node(g_); node != INVALID; ++node) {
      if (nodes_[node] == SOURCE || nodes_[node] == SINK)
        continue;
      if (getNodeInDegree(node) == 1 && getNodeOutDegree(node) == 1) {
        // elementary path, mark node to be removed
        elementaryNodes.push_back(node);
      }
    }
    // handle marked nodes: remove their edges and delete them
    for (auto &node : elementaryNodes) {
      // link before with after
      ListDigraph::OutArcIt outArc(g_, node);
      for (; outArc != INVALID; ++outArc) {
        ListDigraph::InArcIt inArc(g_, node);
        for (; inArc != INVALID; ++inArc) {
          bool found = false;
          // Find existing arc between before and after
          ListDigraph::OutArcIt arc(g_, g_.source(inArc));
          for (; arc != INVALID; ++arc) {
            if (g_.target(arc) == g_.target(outArc)) {
              // a link already exists
              weights_[arc] = 1 - (1 - weights_[arc]) *
                                      (1 - weights_[inArc] * weights_[outArc]);
              found = true;
              break;
            }
          }
          if (!found) { // no existing link.. add one
            ListDigraph::Arc newArc =
                g_.addArc(g_.source(inArc), g_.target(outArc));
            weights_[newArc] = weights_[inArc] * weights_[outArc];
          }
        }
      }
      g_.erase(node);
      changing = true;
    }
  }
}

Cut Graph::CreateFirstCut() {
  auto source = name_to_node_[SOURCE];
  auto target = name_to_node_[SINK];

  Nodes left;
  Nodes middle;
  Nodes right;
  Edges covered;
  // initially all nodes are on the right
  for (ListDigraph::NodeIt node(g_); node != INVALID; ++node) {
    right.set(g_.id(node));
  }
  // move source from right to left
  left.set(g_.id(source));
  right.reset(g_.id(source));
  // move nodes adjacent to source from right to middle
  // and mark covered edges in the process
  for (ListDigraph::OutArcIt arc(g_, source); arc != INVALID; ++arc) {
    int endId = g_.id(g_.target(arc));
    if (endId == g_.id(target)) {
      return Cut();
    }
    covered.set(g_.id(arc));
    middle.set(endId);
    right.reset(endId);
  }
  // mark as covered: the edges from the middle not going to the right
  FOREACH_BS(nodeId, middle) {
    ListDigraph::Node node = g_.nodeFromId(nodeId);
    for (ListDigraph::OutArcIt arc(g_, node); arc != INVALID; ++arc) {
      if (!right[g_.id(g_.target(arc))]) {
        covered.set(g_.id(arc));
      }
    }
  }

  // create the cut and recurse
  return Cut(left, middle, right, covered);
}

void Graph::RemoveRedundantCuts() {
  for (size_t i = 0; i < cuts_.size(); i++) {
    Cut currenti = cuts_.at(i);
    for (size_t j = i + 1; j < cuts_.size(); j++) {
      Cut currentj = cuts_.at(j);
      // see if one of them is contained in the other
      if (currenti.Overlaps(currentj)) {
        cuts_.erase(cuts_.begin() + i);
        i--;
        break;
      }
      if (currentj.Overlaps(currenti)) {
        cuts_.erase(cuts_.begin() + j);
        j--;
      }
    }
  }
}

void Graph::RefineCuts() {
  ListDigraph::Node target = name_to_node_[SINK];
  // check for non-minimality: containment of cuts in other cuts
  RemoveRedundantCuts();

  // grow each cut (if necessary) into a good cut
  vector<Cut> goodCuts;
  for (auto &cut : cuts_) {
    Nodes right = cut.getRight();
    Nodes middle = cut.getMiddle();
    Edges covered = cut.getCoveredEdges();
    // repeat until nothing changes
    while (true) {
      // for each node on the right, make sure its outgoing neighbors are all
      // on the right also
      vector<int> toAdd;
      FOREACH_BS(nodeId, right) {
        ListDigraph::Node node = g_.nodeFromId(nodeId);
        for (ListDigraph::OutArcIt arc(g_, node); arc != INVALID; ++arc) {
          // back edge, grow cut with this node
          if (!right[g_.id(g_.target(arc))]) {
            toAdd.push_back(nodeId);
            break;
          }
        }
      }
      if (toAdd.size() == 0)
        break;
      for (auto nodeId : toAdd) {
        right.reset(nodeId);
        middle.set(nodeId);
      }
    }
    // Now some new edges can be covered due to moving nodes to the middle
    // mark these edges as covered
    FOREACH_BS(nodeId, middle) {
      ListDigraph::Node middleNode = g_.nodeFromId(nodeId);
      for (ListDigraph::OutArcIt arc(g_, middleNode); arc != INVALID; ++arc) {
        if (!right[g_.id(g_.target(arc))]) {
          covered.set(g_.id(arc));
        }
      }
    }
    // add the new good cut
    goodCuts.push_back(Cut(cut.getLeft(), middle, right, covered));
  }
  cuts_ = goodCuts;

  // Minimize good cuts:
  // If a node in the middle group has no outgoing edges to the right group
  // Then move it to the left group
  vector<Cut> bestCuts;
  for (auto &cut : cuts_) {
    Nodes middle = cut.getMiddle();
    Nodes left = cut.getLeft();
    Nodes right = cut.getRight();
    vector<int> toMove;
    FOREACH_BS(nodeId, middle) {
      // make sure it has at least one edge to the right
      bool hasRight = false;
      for (ListDigraph::OutArcIt arc(g_, g_.nodeFromId(nodeId)); arc != INVALID;
           ++arc) {
        if (right[g_.id(g_.target(arc))]) {
          hasRight = true;
          break;
        }
      }
      if (!hasRight) {
        toMove.push_back(nodeId);
      }
    }
    for (auto nodeId : toMove) {
      middle.reset(nodeId);
      left.set(nodeId);
    }
    bestCuts.push_back(Cut(left, middle, right, cut.getCoveredEdges()));
  }
  cuts_ = bestCuts;

  // Last: remove redundant cuts again
  RemoveRedundantCuts();
}

vector<Cut> Graph::FindSomeGoodCuts() {
  ListDigraph::Node target = name_to_node_[SINK];
  // start by forming the first cut: adjacent to source
  Cut firstCut = CreateFirstCut();
  if (firstCut.getMiddle().none()) {
    // That was a dummy returned cut, i.e. no cuts available.
    return {firstCut};
  }

  Nodes currentMiddle = firstCut.getMiddle();
  Nodes currentLeft = firstCut.getLeft();
  Nodes currentRight = firstCut.getRight();
  Edges currentCovered = firstCut.getCoveredEdges();

  cuts_.push_back(firstCut);
  bool added = true;
  while (added) { // repeat until nothing new is added
    Nodes middle = currentMiddle;
    Nodes left = currentLeft;
    Nodes right = currentRight;
    Edges covered = currentCovered;
    added = false;
    FOREACH_BS(nodeId, currentMiddle) {
      ListDigraph::Node node = g_.nodeFromId(nodeId);
      vector<int> nextNodes;
      vector<int> nextArcs;
      for (ListDigraph::OutArcIt arc(g_, node); arc != INVALID; ++arc) {
        ListDigraph::Node next = g_.target(arc);
        int nextId = g_.id(next);
        if (nextId == g_.id(target)) { // node connected to target, ignore all
                                       // of its neighbors
          nextNodes.clear();
          nextArcs.clear();
          break;
        } else if (right[nextId]) { // eligible for moving from right to
                                    // middle
          nextNodes.push_back(nextId);
          nextArcs.push_back(g_.id(arc));
        }
      }
      if (nextNodes.size() > 0) { // There are nodes to move from right to left
        added = true;
        for (auto nextId : nextNodes) {
          right.reset(nextId);
          middle.set(nextId);
        }
        for (auto nextId : nextArcs) {
          covered.set(nextId);
        }
        middle.reset(nodeId);
        left.set(nodeId);
      }
    }
    if (added) {
      // mark as covered: all edges going from the middle not to the right
      FOREACH_BS(nodeId, middle) {
        ListDigraph::Node middleNode = g_.nodeFromId(nodeId);
        for (ListDigraph::OutArcIt arc(g_, middleNode); arc != INVALID; ++arc) {
          if (!right[g_.id(g_.target(arc))]) {
            covered.set(g_.id(arc));
          }
        }
      }
      Cut newCut(left, middle, right, covered);
      cuts_.push_back(newCut);
      currentMiddle = middle;
      currentLeft = left;
      currentRight = right;
      currentCovered = covered;
    }
  }
  RefineCuts();
  return cuts_;
}