#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <limits.h>
#include <vector>
#include <chrono>

// Grafo implementado no formato lista de adjascência:
// - Um vector que representa o conjuto de nós do grafo
// - Cada nó (posição do vector) armazena um ponteiro para o primeiro edge
//      da lista encadeada de out edges referente a ele

class Graph {
public:
  struct edge {
    int destNode = 0;
    double flow = 0, capacity = 0;
    edge *next = nullptr, *reverse = nullptr;
  };

private:
  int m = 0, n = 0;
  edge **G;

  std::vector<int> numEdges;

public:
  Graph(int numV);
  Graph(const Graph &other);
  ~Graph();

  void add_edge_with_reverse(int u, int v, double capacity);
  void add_flow(int u, int v, double flow);
  void add_capacity(int u, int v, double capacityEdit);
  void zera_flow();

  edge *get_edge(int u);
  edge *get_edge(int u, int v);

  int num_vertices();
  int num_edges();
  int get_capacity(int u, int v);
  
  void print_grafo();
};

#endif // GRAPH_H