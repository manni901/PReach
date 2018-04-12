#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include "Util.h"

using namespace std;

void GenerateRandomGraph(int num_nodes, int num_edges, vector<EdgeInfo>& edge_info) {
    vector<pair<int,int>> edge_terminals;
    for(int i = 1; i <= num_nodes; i++) {
        for(int j = i+1; j <= num_nodes; j++) {
            edge_terminals.push_back(make_pair(i, j));
            edge_terminals.push_back(make_pair(j, i));
        }
    }
    srand(time(0));
    random_shuffle(edge_terminals.begin(), edge_terminals.end());
    for(int i = 0; i < num_edges; i++) {
        EdgeInfo info;
        info.edge_terminals = edge_terminals[i];
        info.p = rand() / (RAND_MAX + 1.);
        edge_info.push_back(info);
    }
}

int main(int argc, char** argv) {
    vector<EdgeInfo> edge_info;
    GenerateRandomGraph(atoi(argv[1]), atoi(argv[2]), edge_info);

    ofstream out(argv[3]);
    for(auto& info : edge_info) {
        out << info.edge_terminals.first << " " << info.edge_terminals.second << " " << info.p << "\n";
    }
}