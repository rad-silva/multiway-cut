#ifndef HIGHEST_PUSH_RELABEL
#define HIGHEST_PUSH_RELABEL

#include "graph.hpp"
#include "priority_queue.hpp"
#include "list_node_height.hpp"

#include <vector>

class highest_push_relabel_max_flow {
  Graph Gf;
  int src, sink;
  int num_nodes;

  int d = 0;
  long max_flow = 0;
  long num_pushs = 0;
  long num_relabels = 0;
  long num_gaps = 0;
  long time_execution = 0;
  
  std::vector<double> node_excess;
  std::vector<int> node_height;
  list_node_height list_height;
  PriorityQueueWithRestrictedPush bucket;

  void initializePreflow();
  void pushFlow(int u, Graph::edge *e, double flow);
  void relabel(int u);
  bool isAdmissible(int u, Graph::edge *e);
  void discharge(int u);
  void gap(int k);

public:
  // ctor
  highest_push_relabel_max_flow(Graph &G);

  int solve(int s, int t);

  int get_max_flow() { return max_flow; };
  long get_num_pushs() { return num_pushs; };
  long get_num_relabels() { return num_relabels; };
  long get_num_gaps() { return num_gaps; };
  long get_time_execution() { return time_execution; };

  void add_edge(int u, int v, double cost);

  struct edge { int src; int dst; };
  std::vector<edge> get_edges_cut();

  void print_local();
};

#endif