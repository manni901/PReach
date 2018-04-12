#include <iostream>
#include <fstream>
#include "Term.h"
#include "Polynomial.h"
#include "RandomSolver.h"
#include <cstdlib>
#include <string>

using namespace std;

void GenerateRandomGraph(int num_nodes, int num_edges, vector<EdgeInfo>& edge_info) {
    vector<pair<int,int>> edge_terminals;
    for(int i = 0; i < num_nodes; i++) {
        for(int j = i+1; j < num_nodes; j++) {
            edge_terminals.push_back(make_pair(i, j));
            edge_terminals.push_back(make_pair(j, i));
        }
    }
    srand(time(0));
    random_shuffle(edge_terminals.begin(), edge_terminals.end());
    for(int i = 0; i < num_edges; i++) {
        EdgeInfo info;
        info.edge_terminals = edge_terminals[i];
        srand(time(0));
        info.p = rand() / (RAND_MAX + 1.);
        edge_info.push_back(info);
    }
}

int main(int argc, char** argv) {
    /*if (argc != 4) {
        cout << "Usage: |program_name| |Edge_File| |Source_File| |Target_File|";
        exit(1);
    }*/

    Edges all_edges;
    Nodes source_nodes, target_nodes;
    vector<EdgeInfo> edge_info;
    int val;

    /*int num_nodes = atoi(argv[1]);
    int num_edges = atoi(argv[2]);

    for(int i = 0; i < num_edges; i++) all_edges.set(i);
    source_nodes.set(0);
    target_nodes.set(num_nodes - 1);

    GenerateRandomGraph(num_nodes, num_edges, edge_info);*/

    ifstream in(argv[1]);
    int num_edges;
    in >> num_edges;
    for(int i = 0; i < num_edges; i++) {
        EdgeInfo info;
        in >> info.edge_terminals.first >> info.edge_terminals.second >> info.p;
        edge_info.push_back(move(info));
        all_edges.set(i);
    }
    in.close();

    ifstream in_source(argv[2]);
    int num_sources;
    in_source >> num_sources;
    while(num_sources--) {
        in_source >> val;
        source_nodes.set(val);
    }
    in_source.close();

    ifstream in_target(argv[3]);
    int num_targets;
    in_target >> num_targets;
    while(num_targets--) {
        in_target >> val;
        target_nodes.set(val);
    }
    in_target.close();

    RandomSolver solver(source_nodes, target_nodes, all_edges);
    cout << "Path reachability probability: " << solver.Solve(edge_info) << "\n";
}