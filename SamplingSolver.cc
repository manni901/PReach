#include "SamplingSolver.h"

double SamplingSolver::Solve() {
  double result = 0.0;
  Edges sample_edges;
  if (fixed_) {
    sample_edges = Probe();
  }
  for (int i = 0; i < num_iteration_; i++) {
    Graph graph;
    graph.CopyFrom(graph_);

    //graph.Print();

    auto prob_map = fixed_ ? SampleFixed(sample_edges) : SampleRandom();

    graph.UpdateWeights(prob_map);
    graph.Minimize();

    //graph.Print();
    if (graph.CountArcs() > 1) {
      SausageSolver solver(graph);
      result += solver.Solve();
    } else if (graph.CountArcs() == 1)
      result += 1.0;
    else
      result += 0.0;
  }
  return result / double(num_iteration_);
}

void SamplingSolver::InitRand() {
  timeval time;
  gettimeofday(&time, NULL);
  theRandomMT_.seed((time.tv_sec * 1000) + (time.tv_usec / 1000));
  theRandomGenerator_ = std::uniform_real_distribution<double>(0.0, 1.0);
}

// Returns a map keyed by edge id and values giving the random
// probability for that. Edges with probability greater than
// their weight can then be replaced by 1 and less than by 0.
unordered_map<int, double> SamplingSolver::SampleFixed(Edges &sampleEdges) {
  unordered_map<int, double> edge_prob;
  FOREACH_BS(edge_id, sampleEdges) { edge_prob[edge_id] = NextRand(); }
  return edge_prob;
}

unordered_map<int, double> SamplingSolver::SampleRandom() {
  unordered_map<int, double> edge_prob;
  Edges all_edges = graph_.EdgesAsBitset();
  FOREACH_BS(edge_id, all_edges) {
    if (NextRand() >= success_prob_) {
      edge_prob[edge_id] = NextRand();
    }
  }
  return edge_prob;
}

unordered_map<int, double>
SamplingSolver::SampleWeightedRandom(vector<EdgeSubset> &edge_subsets) {
  unordered_map<int, double> edge_prob;
  Edges sample_edges;
  vector<int> chances = GetChanceVector(edge_subsets);
  int budget = (int)ceil(graph_.CountArcs() * success_prob_);

  while (budget > 0 && chances.size() > 0) {
    int index = (int)floor(NextRand() * chances.size());
    FOREACH_BS(edge_id, edge_subsets[chances[index]].GetEdges()) {
      if (edge_prob.find(edge_id) == edge_prob.end()) {
        edge_prob[edge_id] = NextRand();
        sample_edges.set(edge_id);
        budget--;
        if (budget == 0)
          break;
      }
    }

    int temp = index - 1;
    while (temp >= 0 && chances[temp] == chances[index])
      temp--;
    int start = temp + 1;
    temp = index + 1;
    while (temp < chances.size() && chances[temp] == chances[index])
      temp++;
    int end = temp - 1;
    chances.erase(chances.begin() + start, chances.begin() + end);
  }

  if (budget > 0) {
    Edges all_edges = graph_.EdgesAsBitset();
    Edges remaining_edges = all_edges | ~sample_edges;
    vector<int> remaining_edge_ids;
    FOREACH_BS(id, remaining_edges) remaining_edge_ids.push_back(id);

    while (budget > 0) {
      int edge_id = (int)floor(NextRand() * remaining_edge_ids.size());
      if (edge_prob.find(edge_id) == edge_prob.end()) {
        edge_prob[edge_id] = NextRand();
        budget--;
        if (budget == 0)
          break;
      }
    }
  }
  return edge_prob;
}

vector<int> SamplingSolver::GetChanceVector(vector<EdgeSubset> &edge_subsets) {
  double min_weight = numeric_limits<double>::max();
  double max_weight = 0.0;
  for (auto &subset : edge_subsets) {
    double weight = subset.Weight();
    if (weight < min_weight)
      min_weight = weight;
    if (weight > max_weight)
      max_weight = weight;
  }
  double weight_range = max_weight - min_weight;
  vector<int> chances;
  int index = 0;
  for (auto &subset : edge_subsets) {
    // normalization
    double weight_ratio = (subset.Weight() - min_weight) * 100.0 / weight_range;
    int num_chance = (int)ceil(weight_ratio);
    while (num_chance--) {
      chances.push_back(index);
    }
    index++;
  }
}

Edges SamplingSolver::Probe() {
  vector<EdgeSubset> edge_subsets;
  if (weighted_) {
    unordered_map<int, EdgeInfo> edge_info;
    graph_.GetEdgeInfo(edge_info);
    auto cuts = graph_.FindSomeGoodCuts();
    int id = 0;
    vector<EdgeSubset> edge_subsets;
    for (auto &cut : cuts) {
      edge_subsets.emplace_back(id++, cut, edge_info);
    }
  }

  Edges min_time_edges;
  double min_time = numeric_limits<double>::max();
  for (int i = 0; i < probe_size_; i++) {
    double probe_time = 0.0;
    unordered_map<int, double> prob_map =
        weighted_ ? SampleWeightedRandom(edge_subsets) : SampleRandom();
    Edges sampleEdges;
    for (auto &edge : prob_map) {
      sampleEdges.set(edge.first);
    }
    for (int j = 0; j < probe_repeat_; j++) {
      Graph graph;
      graph.CopyFrom(graph_);
      auto fixed_prob_map = SampleFixed(sampleEdges);
      graph.UpdateWeights(fixed_prob_map);
      graph.Minimize();

      double t_start = GetCPUTime();
      if (graph.CountArcs() > 1) {
        SausageSolver solver(graph);
        double val = solver.Solve();
      }
      double t_end = GetCPUTime();
      probe_time += t_end - t_start;
    }
    if (probe_time < min_time) {
      min_time = probe_time;
      min_time_edges = sampleEdges;
    }
  }
  return min_time_edges;
}