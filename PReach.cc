#include "Polynomial.h"
#include "Term.h"
#include <fstream>
#include <lemon/bfs.h>
#include <lemon/list_graph.h>
#include <sstream>
#include "SausageSolver.h"
#include "RandomSolver.h"
#include <memory>

using namespace std;
using lemon::Bfs;
using lemon::INVALID;
using lemon::ListDigraph;

typedef ListDigraph::ArcMap<double> WeightMap;
typedef ListDigraph::NodeMap<string> NodeNames;
typedef map<string, ListDigraph::Node> NameToNode;

const string SOURCE = "SOURCE";
const string SINK = "SINK";
const string PRE_YES = "pre";
const string PRE_NO = "nopre";

// map names to nodes
ListDigraph::Node FindNode(string name, ListDigraph &g, NodeNames &nMap,
                           NameToNode &nodeMap) {

  // look in map
  if (nodeMap.find(name) == nodeMap.end()) { // not found, add
    ListDigraph::Node node = g.addNode();
    nodeMap[name] = node;
  }

  ListDigraph::Node node = nodeMap[name];
  nMap[node] = name;

  return node;
}

bool EdgeExists(ListDigraph &g, ListDigraph::Node &source,
                ListDigraph::Node &target) {
  for (ListDigraph::OutArcIt arc(g, source); arc != INVALID; ++arc) {
    if (g.id(g.target(arc)) == g.id(target))
      return true;
  }
  return false;
}

// Create the graph from the file
void CreateGraph(char *filename, ListDigraph &g,
                 ListDigraph::NodeMap<string> &nMap, NameToNode &nodeMap,
                 WeightMap &wMap) {
  fstream in(filename);

  while (!in.eof()) {
    string start;
    string stop;
    double weight = -1.0;
    in >> start >> stop >> weight;

    if (weight == -1.0)
      continue;

    ListDigraph::Node sN = FindNode(start, g, nMap, nodeMap);
    ListDigraph::Node tN = FindNode(stop, g, nMap, nodeMap);
    if (!EdgeExists(g, sN, tN)) {
      ListDigraph::Arc a = g.addArc(sN, tN);
      wMap[a] = weight;
    }
  }
  in.close();
}

void ReadList(string filename, vector<string> &list) {
  fstream in(filename.data());
  string item;
  while (!in.eof()) {
    item = "";
    in >> item;

    if (item.empty())
      continue;
    list.push_back(item);
  }
  in.close();
}

/*Reads sources and targets and adds a unified source and unified sink to the
 * graph HOW: adds a new SOURCE node to the graph and a 1.0-weight edge to all
 * sources same with SINK and all targets*/
void UnifyTerminals(ListDigraph &g, WeightMap &wMap, NodeNames &nMap,
                    NameToNode &nodeMap, string sourcesFile,
                    string targetsFile) {
  vector<string> sources;
  vector<string> targets;

  // read sources and targets
  ReadList(sourcesFile, sources);
  ReadList(targetsFile, targets);

  // create unified source and sink nodes
  ListDigraph::Node source = FindNode(SOURCE, g, nMap, nodeMap);
  ListDigraph::Node sink = FindNode(SINK, g, nMap, nodeMap);

  // add an edge from the new source to all sources
  for (auto nodeName : sources) {
    ListDigraph::Node node = FindNode(nodeName, g, nMap, nodeMap);
    if (!EdgeExists(g, source, node)) {
      ListDigraph::Arc arc = g.addArc(source, node);
      wMap[arc] = 1.0;
    }
  }

  // add an edge from all targets to the new sink
  for (auto nodeName : targets) {
    ListDigraph::Node node = FindNode(nodeName, g, nMap, nodeMap);
    if (!EdgeExists(g, node, sink)) {
      ListDigraph::Arc arc = g.addArc(node, sink);
      wMap[arc] = 1.0;
    }
  }
}

/*Gets the In-degree of a node*/
int getNodeInDegree(ListDigraph &g, ListDigraph::Node &node) {
  int count = 0;
  for (ListDigraph::InArcIt arc(g, node); arc != INVALID; ++arc)
    count++;
  return count;
}

/*Gets the Out-degree of a node*/
int getNodeOutDegree(ListDigraph &g, ListDigraph::Node &node) {
  int count = 0;
  for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc)
    count++;
  return count;
}

/*Reverses the graph: replaces each edge by its reverse edge*/
void reverseGraph(ListDigraph &g) {
  // Collect a list of all edges
  vector<ListDigraph::Arc> arcs;
  for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc) {
    arcs.push_back(arc);
  }
  for (auto &arc : arcs) {
    g.reverseArc(arc);
  }
}

/*Removes "Isolated" nodes from the graph
 * An isolated node is the nodes that are not reachable from source or
 * can't reach to sink*/
void RemoveIsolatedNodes(ListDigraph &g, NameToNode &nodeMap) {
  // First: Make a forward traversal and mark the reachable nodes from source
  Nodes forward;
  Bfs<ListDigraph> bfs(g);
  bfs.run(nodeMap[SOURCE]);
  for (ListDigraph::NodeIt node(g); node != INVALID; ++node) {
    if (bfs.reached(node)) {
      forward.set(g.id(node));
    }
  }
  // Second: reverse the graph and make a backward traversal
  // and mark the reachable nodes from the sink
  ListDigraph reverseG;
  Nodes backward;
  reverseGraph(g);
  bfs = Bfs<ListDigraph>(g);
  bfs.run(nodeMap[SINK]);
  for (ListDigraph::NodeIt node(g); node != INVALID; ++node) {
    if (bfs.reached(node)) {
      backward.set(g.id(node));
    }
  }
  // reverse the graph again to return it to original state
  reverseGraph(g);

  // collect bad nodes
  vector<ListDigraph::Node> badNodes;
  for (ListDigraph::NodeIt node(g); node != INVALID; ++node) {
    if (g.id(node) != g.id(nodeMap[SOURCE]) &&
        g.id(node) != g.id(nodeMap[SINK]) &&
        !(forward[g.id(node)] && backward[g.id(node)]))
      badNodes.push_back(node);
  }

  // Erase all bad nodes
  for (auto &node : badNodes) {
    g.erase(node);
  }
}

/*
Removes edges that are self cycles
*/
void RemoveSelfCycles(ListDigraph &g) {
  for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc) {
    if (g.source(arc) == g.target(arc)) {
      g.erase(arc);
    }
  }
}

/*Collapses all elementary paths
 * An elementary path: a --> x --> b , with x not connected to anything else
 * we delete x and create a new link a --> b with weight w =
 * weight(a-->x)*weight(x-->b) if an edge a --> b already exists before with
 * weight w', we merge the old edge with the new one with a weight =
 * 1-(1-w)(1-w')
 * */
void CollapseELementaryPaths(ListDigraph &g, WeightMap &wMap, NodeNames &nMap) {
  // repeat until nothing changes
  bool changing = true;
  while (changing) {
    changing = false;
    RemoveSelfCycles(g);
    vector<ListDigraph::Node> elementaryNodes;
    for (ListDigraph::NodeIt node(g); node != INVALID; ++node) {
      if (nMap[node] == SOURCE || nMap[node] == SINK)
        continue;
      if (getNodeInDegree(g, node) == 1 && getNodeOutDegree(g, node) == 1) {
        // elementary path, mark node to be removed
        elementaryNodes.push_back(node);
      }
    }
    // handle marked nodes: remove their edges and delete them
    for (auto &node : elementaryNodes) {
      // link before with after
      for (ListDigraph::OutArcIt outArc(g, node); outArc != INVALID; ++outArc) {
        for (ListDigraph::InArcIt inArc(g, node); inArc != INVALID; ++inArc) {
          bool found = false;
          // Find existing arc between before and after
          for (ListDigraph::OutArcIt arc(g, g.source(inArc)); arc != INVALID;
               ++arc) {
            if (g.target(arc) == g.target(outArc)) {
              // a link already exists
              wMap[arc] =
                  1 - (1 - wMap[arc]) * (1 - wMap[inArc] * wMap[outArc]);
              found = true;
              break;
            }
          }
          if (!found) { // no existing link.. add one
            ListDigraph::Arc newArc =
                g.addArc(g.source(inArc), g.target(outArc));
            wMap[newArc] = wMap[inArc] * wMap[outArc];
          }
        }
      }
      g.erase(node);
      changing = true;
    }
  }
}

/*Does the needed preprocessing of the graph:
 * adding source & sink
 * remove isolated nodes
 * collapse elementary paths
 * see comments of each function for details
 * */
void Preprocess(ListDigraph &g, WeightMap &wMap, NodeNames &nMap,
                NameToNode &nodeMap, string sourcesFile, string targetsFile,
                string pre) {
  UnifyTerminals(g, wMap, nMap, nodeMap, sourcesFile, targetsFile);
  if (pre == PRE_YES) {
    RemoveIsolatedNodes(g, nodeMap);
    CollapseELementaryPaths(g, wMap, nMap);
    RemoveSelfCycles(g);
  }
  // EXTRA STEP: make sure source and sink are not directly connected
  ListDigraph::Node source = nodeMap[SOURCE];
  ListDigraph::Node sink = nodeMap[SINK];
  for (ListDigraph::OutArcIt arc(g, source); arc != INVALID; ++arc) {
    if (g.id(g.target(arc)) == g.id(sink)) { // direct source-sink connection -
                                             // isolate with a middle node
      ListDigraph::Node isolator = g.addNode();
      nodeMap["ISOLATOR"] = isolator;
      nMap[isolator] = "ISOLATOR";
      ListDigraph::Arc head = g.addArc(isolator, sink);
      wMap[head] = wMap[arc];
      ListDigraph::Arc tail = g.addArc(source, isolator);
      wMap[tail] = 1.0;
      g.erase(arc);
      break;
    }
  }
}

/*removes cuts that are masked by smaller cuts*/
void RemoveRedundantCuts(vector<Cut> &cuts) {
  for (size_t i = 0; i < cuts.size(); i++) {
    Cut currenti = cuts.at(i);
    for (size_t j = i + 1; j < cuts.size(); j++) {
      Cut currentj = cuts.at(j);
      // see if one of them is contained in the other
      if ((~currenti.getMiddle() & currentj.getMiddle()).none()) {
        // j contained in i: delete i and break
        cuts.erase(cuts.begin() + i);
        i--;
        break;
      }
      if ((~currentj.getMiddle() & currenti.getMiddle()).none()) {
        // i contained in j: delete j
        cuts.erase(cuts.begin() + j);
        j--;
      }
    }
  }
}

/*Minimizes the cuts, then makes sure they are "Good"*/
void RefineCuts(vector<Cut> &cuts, ListDigraph &g, ListDigraph::Node &target) {
  // check for non-minimality: containment of cuts in other cuts
  // RemoveRedundantCuts(cuts);

  // grow each cut (if necessary) into a good cut
  vector<Cut> goodCuts;
  for (auto &cut : cuts) {
    Nodes right = cut.getRight();
    Nodes middle = cut.getMiddle();
    Edges covered = cut.getCoveredEdges();
    // repeat until nothing changes
    while (true) {
      // for each node on the right, make sure its outgoing neighbors are all on
      // the right also
      vector<int> toAdd;
      FOREACH_BS(nodeId, right) {
        ListDigraph::Node node = g.nodeFromId(nodeId);
        for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc) {
          if (!right[g.id(
                  g.target(arc))]) { // back edge, grow cut with this node
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
      ListDigraph::Node middleNode = g.nodeFromId(nodeId);
      for (ListDigraph::OutArcIt arc(g, middleNode); arc != INVALID; ++arc) {
        if (!right[g.id(g.target(arc))]) {
          covered.set(g.id(arc));
        }
      }
    }
    // add the new good cut
    goodCuts.push_back(Cut(cut.getLeft(), middle, right, covered));
  }
  cuts = goodCuts;

  // Minimize good cuts:
  // If a node in the middle group has no outgoing edges to the right group
  // Then move it to the left group
  vector<Cut> bestCuts;
  for (auto &cut : cuts) {
    Nodes middle = cut.getMiddle();
    Nodes left = cut.getLeft();
    Nodes right = cut.getRight();
    vector<int> toMove;
    FOREACH_BS(nodeId, middle) {
      // make sure it has at least one edge to the right
      bool hasRight = false;
      for (ListDigraph::OutArcIt arc(g, g.nodeFromId(nodeId)); arc != INVALID;
           ++arc) {
        if (right[g.id(g.target(arc))]) {
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
  cuts = bestCuts;

  // Last: remove redundant cuts again
  RemoveRedundantCuts(cuts);
}

/*Prints the graph in node IDs*/
void PrintGraph(ListDigraph &g) {
  for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc) {
    cout << g.id(g.source(arc)) << " " << g.id(g.target(arc)) << endl;
  }
}

/*Starting from a vertex cut, recursively finds all other cuts to the right*/
void FindAllCuts(Cut &currentCut, vector<Cut> &cuts, ListDigraph &g,
                 ListDigraph::Node target) {
  Nodes currentMiddle = currentCut.getMiddle();
  Nodes currentLeft = currentCut.getLeft();
  Nodes currentRight = currentCut.getRight();
  Edges currentCovered = currentCut.getCoveredEdges();

  // Go through all nodes in the cut, try to replace it with neighbors
  FOREACH_BS(nodeId, currentMiddle) {
    Nodes middle = currentMiddle;
    Nodes left = currentLeft;
    Nodes right = currentRight;
    Edges covered = currentCovered;
    ListDigraph::Node node = g.nodeFromId(nodeId);
    // Loop over all neighbors of the node
    bool added = false;
    for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc) {
      ListDigraph::Node next = g.target(arc);
      int nextId = g.id(next);
      if (nextId == g.id(target)) { // cut connected to target, STOP HERE
        added = false;
        break;
      } else if (right[nextId]) { // add from right to middle
        added = true;
        right.reset(nextId);
        middle.set(nextId);
        covered.set(g.id(arc));
      }
    }
    if (added) { // added at least one node to the cut
      middle.reset(nodeId);
      left.set(nodeId);
      // mark as covered: all edges going from the middle not to the right
      FOREACH_BS(nodeId, middle) {
        ListDigraph::Node middleNode = g.nodeFromId(nodeId);
        for (ListDigraph::OutArcIt arc(g, middleNode); arc != INVALID; ++arc) {
          if (!right[g.id(g.target(arc))]) {
            covered.set(g.id(arc));
          }
        }
      }
      Cut newCut(left, middle, right, covered);
      cuts.push_back(newCut);
      FindAllCuts(newCut, cuts, g, target);
    }
  }
}

/*prints a cut*/
void PrintCut(Cut &cut, ListDigraph &g) {
  Nodes nodes = cut.getMiddle();
  FOREACH_BS(id, nodes) { cout << id << " "; }
  cout << " : ";
  Edges covered = cut.getCoveredEdges();
  FOREACH_BS(id, covered) {
    cout << g.id(g.source(g.arcFromId(id))) << "-"
         << g.id(g.target(g.arcFromId(id))) << " ";
  }
  cout << endl;
}

/*prints cuts*/
void PrintCuts(vector<Cut> &cuts, ListDigraph &g) {
  for (auto &cut : cuts) {
    PrintCut(cut, g);
  }
}

/*creates first level cut: nodes adjacent to source*/
Cut createFirstCut(ListDigraph &g, ListDigraph::Node source,
                   ListDigraph::Node target) {
  Nodes left;
  Nodes middle;
  Nodes right;
  Edges covered;
  // initially all nodes are on the right
  for (ListDigraph::NodeIt node(g); node != INVALID; ++node) {
    right.set(g.id(node));
  }
  // move source from right to left
  left.set(g.id(source));
  right.reset(g.id(source));
  // move nodes adjacent to source from right to middle
  // and mark covered edges in the process
  for (ListDigraph::OutArcIt arc(g, source); arc != INVALID; ++arc) {
    int endId = g.id(g.target(arc));
    if (endId == g.id(target)) {
      return Cut();
    }
    covered.set(g.id(arc));
    middle.set(endId);
    right.reset(endId);
  }
  // mark as covered: the edges from the middle not going to the right
  FOREACH_BS(nodeId, middle) {
    ListDigraph::Node node = g.nodeFromId(nodeId);
    for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc) {
      if (!right[g.id(g.target(arc))]) {
        covered.set(g.id(arc));
      }
    }
  }

  // create the cut and recurse
  return Cut(left, middle, right, covered);
}

/*Finds *SOME* good cuts: steps from a cut to the next by
replacing every node by all of its neighbors*/
void FindSomeGoodCuts(ListDigraph &g, ListDigraph::Node source,
                      ListDigraph::Node target, vector<Cut> &cuts) {
  // start by forming the first cut: adjacent to source
  Cut firstCut = createFirstCut(g, source, target);
  if (firstCut.getMiddle()
          .none()) { // That was a dummy returned cut, i.e. no cuts available
    return;
  }
  Nodes currentMiddle = firstCut.getMiddle();
  Nodes currentLeft = firstCut.getLeft();
  Nodes currentRight = firstCut.getRight();
  Edges currentCovered = firstCut.getCoveredEdges();
  cuts.push_back(firstCut);
  bool added = true;
  while (added) { // repeat until nothing new is added
    Nodes middle = currentMiddle;
    Nodes left = currentLeft;
    Nodes right = currentRight;
    Edges covered = currentCovered;
    added = false;
    FOREACH_BS(nodeId, currentMiddle) {
      ListDigraph::Node node = g.nodeFromId(nodeId);
      vector<int> nextNodes;
      vector<int> nextArcs;
      for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc) {
        ListDigraph::Node next = g.target(arc);
        int nextId = g.id(next);
        if (nextId == g.id(target)) { // node connected to target, ignore all of
                                      // its neighbors
          nextNodes.clear();
          nextArcs.clear();
          break;
        } else if (right[nextId]) { // eligible for moving from right to middle
          nextNodes.push_back(nextId);
          nextArcs.push_back(g.id(arc));
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
        ListDigraph::Node middleNode = g.nodeFromId(nodeId);
        for (ListDigraph::OutArcIt arc(g, middleNode); arc != INVALID; ++arc) {
          if (!right[g.id(g.target(arc))]) {
            covered.set(g.id(arc));
          }
        }
      }
      Cut newCut(left, middle, right, covered);
      cuts.push_back(newCut);
      currentMiddle = middle;
      currentLeft = left;
      currentRight = right;
      currentCovered = covered;
    }
  }
  // cout << "Before refine: " << cuts.size() << " cuts" << endl;
  // PrintCuts(cuts, g);
  // refine the cuts: minimize and make them good cuts
  RefineCuts(cuts, g, target);
  // cout << "After refine: " << cuts.size() << " cuts" << endl;
  cout << cuts.size() << "  ";
  // PrintCuts(cuts, g);
}

/*Finds all good cuts in g*/
void FindGoodCuts(ListDigraph &g, ListDigraph::Node source,
                  ListDigraph::Node target, vector<Cut> &cuts) {
  // start by forming the first cut: adjacent to source, and recurse
  Cut firstCut = createFirstCut(g, source, target);
  if (firstCut.getMiddle()
          .none()) { // That was a dummy returned cut, i.e. no cuts available
    return;
  }
  cuts.push_back(firstCut);
  FindAllCuts(firstCut, cuts, g, target);

  // cout << "Before refine: " << cuts.size() << " cuts" << endl;
  // PrintCuts(cuts, g);
  // refine the cuts: minimize and make them good cuts
  RefineCuts(cuts, g, target);
  // cout << "After refine: " << cuts.size() << " cuts" << endl;
  cout << cuts.size() << "  ";
  // PrintCuts(cuts, g);
}

/*Gets all edges as a bitset*/
void EdgesAsBitset(ListDigraph &g, Edges &edges) {
  for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc) {
    edges.set(g.id(arc));
  }
}

/*Removes cuts that are obsoleted by cut
A cut is obsolete if its middle set overlaps with the left set of cut*/
void RemoveObsoleteCuts(vector<Cut> &cuts, Cut &cut) {
  for (size_t i = 0; i < cuts.size(); i++) {
    Cut currentCut = cuts.at(i);
    if ((currentCut.getMiddle() & cut.getLeft())
            .any()) { // currentCut is obsolete
      cuts.erase(cuts.begin() + i);
      i--;
    }
  }
}

void ConsumeSausage(ListDigraph &g, WeightMap &wMap, Polynomial &poly,
                    Edges &sausage, Nodes &endNodes) {
  // Build a dictionary of edgeId -> source and target node ids
  // Will need it with each collapsation operation within this sausage
  unordered_map<int, pair<int, int>> edgeTerminals;
  FOREACH_BS(edgeId, sausage) {
    pair<int, int> terminals;
    ListDigraph::Arc arc = g.arcFromId(edgeId);
    terminals.first = g.id(g.source(arc));
    terminals.second = g.id(g.target(arc));
    edgeTerminals[edgeId] = terminals;
  }

  // start adding the edges in the current sausage
  // here we collapse after each addition (arbitrary)
  int edgeCounter = 0;
  FOREACH_BS(edgeId, sausage) {
    edgeCounter++;
    // cout << "Adding edge " << edgeCounter;
    poly.AddEdge(edgeId, wMap[g.arcFromId(edgeId)]);
    // cout << ", Collapsing!" << endl;
    poly.Collapse(sausage, endNodes, edgeTerminals);
  }

  // Advance the polynomial: make it ready for next sausage
  poly.Advance();
}

/*Just for debugging - ignore*/
bool compareCuts(Cut cut1, Cut cut2) {
  return (cut1.getMiddle().count() < cut2.getMiddle().count());
}

/*Finds reachability probability given the vertex cuts
    Starts from the source to the first cut
    Then removes all the obsolete cuts and pick a next cut
    An obsolete cut is a cut whose middle set intersects with
    the left set of the current cut*/
double Solve(ListDigraph &g, WeightMap &wMap, NameToNode &nodeMap,
             vector<Cut> &cuts) {
  // FOR DEBUGGING - REMOVE
  // sort(cuts.begin(), cuts.end(), compareCuts);

  // set up the source term and start the polynomial
  ListDigraph::Node source = nodeMap[SOURCE];
  Nodes zSource, wSource;
  zSource.set(g.id(source));
  Polynomial poly(zSource);

  Edges covered; // This will hold the set of covered edges so far
  Edges sausage; // This will hold the current sausage: edges being considered
                 // for addition

  // repeat until no cuts left
  while (cuts.size() > 0) {
    // select a cut: here we just select the first one (arbitrary)
    Cut nextCut = cuts.front();
    // cout << "Available " << cuts.size() << " cuts, Using cut with size " <<
    // nextCut.size();
    cout << nextCut.size() << "  ";
    cuts.erase(cuts.begin());
    // Identify the sausage: The current set of edges in question
    sausage = nextCut.getCoveredEdges() & ~covered;
    // cout << ", Sausage size: " << sausage.count() << endl;
    cout << sausage.count() << "  ";
    // Consume the current sausage
    try {
      ConsumeSausage(g, wMap, poly, sausage, nextCut.getMiddle());
    } catch (exception &e) {
      cout << endl
           << "EXCEPTION: " << e.what() << ": " << typeid(e).name() << endl;
      exit(3);
    }
    // mark the sausage as covered
    covered |= sausage;
    // remove obsolete cuts
    RemoveObsoleteCuts(cuts, nextCut);
  }

  // Last: add the edges between the last cut and the target node
  Edges allEdges; // set of all edges in the network
  EdgesAsBitset(g, allEdges);
  sausage = allEdges &
            ~covered; // the last sausage is all edges that are not yet covered
  Nodes targetSet;    // The last stop
  targetSet.set(g.id(nodeMap[SINK]));
  // cout << "Last step, Sausage size: " << sausage.count() << endl;
  cout << "1  " << sausage.count() << "  ";
  ConsumeSausage(g, wMap, poly, sausage, targetSet);

  // RESULT
  return poly.GetResult();
  // return -1.0;
}

string joinString(vector<string> &parts, string delim) {
  stringstream ss;
  for (size_t i = 0; i < parts.size(); ++i) {
    if (i != 0)
      ss << delim;
    ss << parts[i];
  }
  return ss.str();
}

void splitString(string str, vector<string> &result, char delim) {
  istringstream stream(str);
  while (!stream.eof()) {
    string part;
    getline(stream, part, delim);
    result.push_back(part);
  }
}

string arcToString(ListDigraph &g, WeightMap &wMap, NodeNames &nNames,
                   ListDigraph::Arc &arc) {
  stringstream ss;
  ss << nNames[g.source(arc)];
  ss << ">";
  ss << nNames[g.target(arc)];
  ss << ">";
  stringstream ws;
  ws << wMap[arc];
  string wString = ws.str();
  if (wString.length() > 6) {
    wString.resize(6);
  }
  ss << wString;
  return ss.str();
}

string edgesToReferenceString(ListDigraph &g, WeightMap &wMap,
                              NodeNames &nNames) {
  vector<string> edges;
  for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc) {
    string arcString = arcToString(g, wMap, nNames, arc);
    edges.push_back(arcString);
  }
  stringstream ss;
  ss << "\"";
  ss << joinString(edges, "#");
  ss << "\"";
  return ss.str();
}

bool CheckProcessedReference(ListDigraph &g, WeightMap &wMap, NodeNames &nNames,
                             string reference) {
  vector<string> edges;
  // The reference string has to have leading and trailing quotes
  // reference.erase(0,1);
  // reference.erase(reference.length()-1, 1);
  splitString(reference, edges, '#');
  for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc) {
    string arcString = arcToString(g, wMap, nNames, arc);
    vector<string>::iterator it = find(edges.begin(), edges.end(), arcString);
    if (it == edges.end()) {
      return false;
    } else {
      edges.erase(it);
    }
  }
  if (edges.size() == 0)
    return true;
  else {
    return false;
  }
}

int main(int argc, char **argv) {
  if (argc < 5) {
    // arg1: network file
    // arg2: sources file
    // arg3: targets file
    // arg4: method (random or sausage)
    cout << "Usage: preach graph-file sources-file targets-file" << endl;
    return -1;
  }

  ListDigraph g;
  WeightMap wMap(g);   // keeps track of weights
  NodeNames nNames(g); // node names
  NameToNode nodeMap;  // mapping from names to nodes in the graph

  CreateGraph(argv[1], g, nNames, nodeMap, wMap);
  int numNodes = countNodes(g);
  int numEdges = countArcs(g);
  // cout << endl << "Original graph size: " << numNodes << " nodes, " <<
  // numEdges << " edges" << endl;
  cout << numNodes << "  " << numEdges << "  ";

  // Read sources and targets and preprocess
  Preprocess(g, wMap, nNames, nodeMap, argv[2], argv[3], PRE_YES);

  numNodes = countNodes(g);
  numEdges = countArcs(g);
  // cout << endl << "Modified graph size: " << numNodes << " nodes, " <<
  // numEdges << " edges" << endl << endl;
  cout << numNodes << "  " << numEdges << "  ";

  if (numEdges == 0) { // empty graph - source and target unreachable
                       // print the edges for reference
    cout << edgesToReferenceString(g, wMap, nNames) << "  ";

    // cout << ">>0.0" << endl;
    cout << "0.0" << endl;
    return 0;
  }

  ListDigraph::Node source = FindNode(SOURCE, g, nNames, nodeMap);
  ListDigraph::Node target = FindNode(SINK, g, nNames, nodeMap);

  Edges all_edges;
  EdgesAsBitset(g, all_edges);

  Nodes source_bitset, target_bitset;
  source_bitset.set(g.id(source));
  target_bitset.set(g.id(target));

  unordered_map<int, EdgeInfo> edge_info;
  FOREACH_BS(edgeId, all_edges) {
    pair<int, int> terminals;
    ListDigraph::Arc arc = g.arcFromId(edgeId);
    terminals.first = g.id(g.source(arc));
    terminals.second = g.id(g.target(arc));
    edge_info[edgeId].edge_terminals = terminals;
    edge_info[edgeId].p = wMap[arc];
  }

  vector<Cut> cuts;
  // FindGoodCuts(g, source, target, cuts);
  FindSomeGoodCuts(g, source, target, cuts);

  unique_ptr<Solver> solver;
  string choice(argv[4]);

  if (choice == "random") {
      solver = make_unique<RandomSolver>(source_bitset, target_bitset, edge_info);
  } else {
      solver = make_unique<SausageSolver>(source_bitset, target_bitset, edge_info, cuts);
  }

  solver->SetAllEdges(all_edges);

  double prob = solver->Solve();

  cout << prob;
  cout << endl;
  return 0;
}