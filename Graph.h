#ifndef GRAPH_H
#define GRAPH_H 

#include <lemon/bfs.h>
#include <lemon/list_graph.h>

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

class Graph {

}

#endif