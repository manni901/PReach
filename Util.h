#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <bitset>
#include <unordered_map>

#define NUM_EDGES 1024
#define NUM_NODES 512

typedef std::bitset<NUM_EDGES> Edges;
typedef std::bitset<NUM_NODES> Nodes;
typedef std::unordered_map<int, std::pair<int, int>> EDGE_INFO;

#define FOREACH_BS(v, vSet)	  \
	for (size_t v=vSet._Find_first(); v!=vSet.size(); v=vSet._Find_next(v))

struct EdgeInfo {
  double p;
  std::pair<int, int> edge_terminals;
};

#endif