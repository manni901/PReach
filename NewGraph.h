#ifndef NEW_GRAPH_H
#define NEW_GRAPH_H

#include "Util.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;

struct EInfo {
  double p;
  int source;
  int target;

  EInfo(int s, int t, double p) : source(s), target(t), p(p) {}
};

class NewGraph {
public:
  NewGraph() {
    out_edges_.resize(NUM_NODES);
    in_edges_.resize(NUM_NODES);
  }
  void AddNode(int i) { nodes_.set(i); }

  void AddEdge(int source, int target, double p) {
    AddNode(source);
    AddNode(target);
    edge_info_.emplace_back(source, target, p);
    edges_.set(curr_edge_);
    out_edges_[source].insert(curr_edge_);
    in_edges_[target].insert(curr_edge_);
    curr_edge_++;
  }

  void Reverse() {
    for (auto &info : edge_info_) {
      int temp = info.source;
      info.source = info.target;
      info.target = temp;
    }
    auto temp = out_edges_;
    out_edges_ = in_edges_;
    in_edges_ = temp;
  }

  void Create(string &file_name) {
    ifstream in(file_name);
    int max = 0;
    while (!in.eof()) {
      int start;
      int stop;
      double weight = -1.0;
      in >> start >> stop >> weight;
      if (start > max)
        max = start;
      if (stop > max)
        max = stop;

      if (weight == -1.0)
        continue;

      AddEdge(start, stop, weight);
    }
    source_ = max + 1;
    target_ = max + 2;
    in.close();
  }

  void RemoveEdge(int i) {
    edges_.reset(i);
    out_edges_[edge_info_[i].source].erase(i);
    in_edges_[edge_info_[i].target].erase(i);
  }

  void RemoveSelfCycles() {
    for (int i = 0; i < edge_info_.size(); i++) {
      if (edge_info_[i].source == edge_info_[i].target) {
        RemoveEdge(i);
      }
    }
  }

  int GetNodeInDegree(int node_id) { return in_edges_[node_id].size(); }

  int GetNodeOutDegree(int node_id) { return out_edges_[node_id].size(); }

  vector<int> ReadList(string file_name) {
    vector<int> list;
    ifstream in(file_name);
    int item;
    while (in >> item) {
      list.push_back(item);
    }
    in.close();
    return list;
  }

  void UnifyTerminals(string &sourcesFile, string &targetsFile) {
    vector<int> sources = ReadList(sourcesFile);
    vector<int> targets = ReadList(targetsFile);    

    // add an edge from the new source to all sources
    for (auto node_name : sources) {
      AddEdge(source_, node_name, 1.0);
    }

    // add an edge from all targets to the new sink
    for (auto node_name : targets) {
      AddEdge(node_name, target_, 1.0);
    }
  }

private:
  int curr_edge_ = 0;
  Nodes nodes_;
  Edges edges_;
  vector<EInfo> edge_info_;
  vector<unordered_set<int>> out_edges_;
  vector<unordered_set<int>> in_edges_;
  int source_;
  int target_;
};

#endif