#include <iostream>
#include <fstream>
#include "Term.h"
#include "Polynomial.h"
#include "RandomSolver.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc != 4) {
        cout << "Usage: |program_name| |Edge_File| |Source_File| |Target_File|";
        exit(1);
    }

    Edges all_edges;
    Nodes source_nodes, target_nodes;
    vector<EdgeInfo> edge_info;
    int val;

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
    cout << solver.Solve(edge_info);
}