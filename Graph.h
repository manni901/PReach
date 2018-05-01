#ifndef GRAPH_H
#define GRAPH_H

#include "Cut.h"
#include "Util.h"
#include <fstream>
#include <iostream>
#include <lemon/bfs.h>
#include <lemon/list_graph.h>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;
using lemon::Bfs;
using lemon::INVALID;
using lemon::ListDigraph;

typedef ListDigraph::ArcMap<double> WeightMap;
typedef ListDigraph::ArcMap<string> ArcMap;
typedef ListDigraph::NodeMap<string> NodeNames;
typedef map<string, ListDigraph::Node> NameToNode;

const string SOURCE = "SOURCE";
const string SINK = "SINK";
const string PRE_YES = "pre";
const string PRE_NO = "nopre";

class Graph {

public:
  Graph(string file_name) : weights_(g_), nodes_(g_) { Create(file_name); }

  Graph() : weights_(g_), nodes_(g_) {}

  void CopyFrom(const Graph &graph);

  int CountNodes() { return countNodes(g_); }

  int CountArcs() { return countArcs(g_); }

  // Does the needed preprocessing of the graph:
  // adding source & sink
  // remove isolated nodes
  // collapse elementary paths
  // see comments of each function for details
  void Preprocess(string sources_file, string targets_file, string pre);

  void Minimize();

  void UpdateWeights(unordered_map<int, double> edge_weights);

  // Gets all edges as a bitset.
  Edges EdgesAsBitset();

  void GetEdgeInfo(unordered_map<int, EdgeInfo> &edge_info);

  ListDigraph::Node GetNode(string name);

  ListDigraph &GetInnerG() { return g_; }

  // Finds *SOME* good cuts: steps from a cut to the next by
  // replacing every node by all of its neighbors.
  vector<Cut> FindSomeGoodCuts();

  Nodes GetNodeBitset(string node_name);

  void Print();

private:
  ListDigraph g_;
  WeightMap weights_;
  NodeNames nodes_;
  NameToNode name_to_node_;

  unordered_set<string> edges_;

  vector<Cut> cuts_;

  void Create(string &file_name);

  void ReadList(string file_name, vector<string> &list);

  /*Reads sources and targets and adds a unified source and unified sink to the
   * graph HOW: adds a new SOURCE node to the graph and a 1.0-weight edge to all
   * sources same with SINK and all targets*/
  void UnifyTerminals(string &sourcesFile, string &targetsFile) {
    vector<string> sources;
    vector<string> targets;

    // read sources and targets
    ReadList(sourcesFile, sources);
    ReadList(targetsFile, targets);

    // add an edge from the new source to all sources
    for (auto node_name : sources) {
      AddEdge(SOURCE, node_name, 1.0);
    }

    // add an edge from all targets to the new sink
    for (auto node_name : targets) {
      AddEdge(node_name, SINK, 1.0);
    }
  }

  void AddEdge(string source, string target, double weight);

  // Reverses the graph: replaces each edge by its reverse edge.
  void Reverse();

  // Removes "Isolated" nodes from the graph
  // An isolated node is the nodes that are not reachable from source or
  // can't reach to sink.
  void RemoveIsolatedNodes();

  // Removes edges that are self cycles
  void RemoveSelfCycles();

  /*Gets the In-degree of a node*/
  int getNodeInDegree(ListDigraph::Node &node) {
    int count = 0;
    for (ListDigraph::InArcIt arc(g_, node); arc != INVALID; ++arc)
      count++;
    return count;
  }

  /*Gets the Out-degree of a node*/
  int getNodeOutDegree(ListDigraph::Node &node) {
    int count = 0;
    for (ListDigraph::OutArcIt arc(g_, node); arc != INVALID; ++arc)
      count++;
    return count;
  }

  // Collapses all elementary paths
  // An elementary path: a --> x --> b , with x not connected to anything else
  // we delete x and create a new link a --> b with weight w =
  // weight(a-->x)*weight(x-->b) if an edge a --> b already exists before with
  // weight w', we merge the old edge with the new one with a weight =
  // 1-(1-w)(1-w')
  void CollapseELementaryPaths();

  // creates first level cut: nodes adjacent to source*/
  Cut CreateFirstCut();

  // Minimizes the cuts, then makes sure they are "Good"*/
  void RefineCuts();

  // removes cuts that are masked by smaller cuts*/
  void RemoveRedundantCuts();
};

#endif