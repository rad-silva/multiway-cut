#include "graph.hpp"

#include <iostream>
#include <fstream> 
#include <limits.h>
#include <string.h>
#include <vector>
#include <chrono>
#include <iomanip>


// Inicializa um grafo sem arcos
Graph::Graph(int numV) :
  m(0),
  n(numV),
  G(nullptr),
  numEdges(n+1, 0)
{
  G = new edge*[n + 1];

  for (int u = 0; u <= n; u++) {
    G[u] = nullptr;
    numEdges[u] = 0;
  }
}

// Construtor de cópia
Graph::Graph(const Graph &other) :
  m(other.m),
  n(other.n),
  G(nullptr),
  numEdges(other.numEdges)
{
  // Aloca espaço para os nós do grafo
  G = new edge*[n + 1];

  // Inicializa os ponteiros dos nós
  for (int u = 0; u <= n; u++) {
    G[u] = nullptr;
  }

  // Copia os arcos do grafo original para o grafo atual
  for (int u = 0; u <= n; u++) {
    edge *e;

    for (e = other.G[u]; e != nullptr; e = e->next) {
      add_edge_with_reverse(u, e->destNode, e->capacity, e->index);
    }
  }
}

// Adiciona um arco com origem u, destino v, e capacidade capacity
// e seu reverso como origem v, destino u, e capacidade 0
void Graph::add_edge_with_reverse(int u, int v, double capacity, unsigned index) 
{
  edge *e;

  // verifica o nó que possui menos arcos saindo
  // e realiza um busca pelo arco
  if(numEdges[u] <= numEdges[v]) {
    e = this->get_edge(u, v);
  } else {
    e = this->get_edge(v, u);
    if (e != nullptr)
      e = e->reverse;
  }

  // o arco já existe
  if (e != nullptr) { 
    e->capacity = capacity; // atualiza a capacidade
  } 
  else {   // o arco não existe
    // aloca e adiciona o arco (u,v) na lista de adjacencia de u
    edge *newEdge = new edge;
    newEdge->destNode = v;
    newEdge->capacity = capacity;
    newEdge->index = index;

    newEdge->next = G[u];
    G[u] = newEdge;

    // aloca e adiciona o arco (v,u) na lista de adjacencia de v
    edge *reverseEdge = new edge;
    reverseEdge->destNode = u;
    reverseEdge->capacity = capacity;
    reverseEdge->index = index;

    reverseEdge->next = G[v];
    G[v] = reverseEdge;

    // liga os ponteiros dos arcos direto e reverso
    newEdge->reverse = reverseEdge;
    reverseEdge->reverse = newEdge;

    numEdges[u] = numEdges[u] + 1;
    numEdges[v] = numEdges[v] + 1;
    
    m += 2;
  }
}

void Graph::remove_edge_with_reverse(int u, int v) {
  // Encontrar e remover o arco (u, v) da lista de adjacência de u
  edge *prev_e = nullptr;
  edge *curr_e = G[u];

  while (curr_e != nullptr && curr_e->destNode != v) {
    prev_e = curr_e;
    curr_e = curr_e->next;
  }

  if (curr_e != nullptr) {
    if (prev_e != nullptr)
      prev_e->next = curr_e->next;
    else // curr_e é o primeiro arco da lista u
      G[u] = curr_e->next;

    delete curr_e;
  }

  // Encontrar e remover o arco reverso (v, u) da lista de adjacência de v
  prev_e = nullptr;
  curr_e = G[v];

  while (curr_e != nullptr && curr_e->destNode != u) {
    prev_e = curr_e;
    curr_e = curr_e->next;
  }

  if (curr_e != nullptr) {
    if (prev_e != nullptr)
      prev_e->next = curr_e->next;
    else // curr_e é o primeiro arco da lista de v
      G[v] = curr_e->next;

    delete curr_e;
  }
}

// Acrescenta flow unidades no fluxo atual do arco (u,v)
void Graph::add_flow(int u, int v, double flow) 
{
  edge *e = get_edge(u, v);

  if (e != nullptr and e->flow + flow <= e->capacity) {
    e->flow += flow;
  }
}

// Acrescenta capacityEdit unidades na capacidade atual do arco (u,v)
void Graph::add_capacity(int u, int v, double capacityEdit) 
{
  edge *e = get_edge(u, v);
  
  if (e != nullptr) {
    e->capacity += capacityEdit;
  }
}


// Percorre o grafo definindo todo fluxo como zero
void Graph::zera_flow() 
{
  for (int u = 0; u <= n; u++) {
    edge *e = get_edge(u);
    
    while (e != nullptr) {
      e->flow = 0;
      e = e->next;
    }
  }
}

// Retorna um apontador para o primeiro arco saindo de u
Graph::edge * Graph::get_edge(int u) 
{ 
  return G[u]; 
}

// Retorna um apontador para o arco (u,v), caso exista, ou para nullptr
Graph::edge * Graph::get_edge(int u, int v)
{
  edge *e = G[u];

  while (e != nullptr) {
    if(e->destNode == v)
      return e;
    e = e->next;
  }

  return nullptr;
}


// Retorna o valor da capacidade do arco (u,v)
int Graph::get_capacity(int u, int v)
{
  edge *e = get_edge(u, v);
  
  if (e != nullptr) {
    return (e->capacity);
  }

  return -1;
}

// Retorna o número de vértices contido no grafo
int Graph::num_vertices()
{ 
  return this->n;
}

// Retorna o número de Arcos contido no grafo
int Graph::num_edges() 
{ 
  return this->m; 
}


void Graph::print_grafo() 
{
  std::cout
  << "Nodes: " << n << std::endl
  << "Edges: " << m << std::endl;

  std::cout
  << std::setw(6) << "src"
  << std::setw(6) << "dest"
  << std::setw(6) << "flow"
  << std::setw(6) << "cost" << std::endl;

  for (int u = 0; u <= n; u++) {
    for (edge *e = G[u]; e != nullptr; e = e->next) {
      std::cout
      << std::setw(6) << u
      << std::setw(6) << e->destNode
      << std::setw(6) << e->flow
      << std::setw(18) << e->capacity << std::endl;
    }
  }
  std::cout << "\n";
}

Graph::~Graph()
{
  if (G != nullptr) {
    for (int u = 0; u <= n; u++) {
      edge *e_temp = G[u];

      while (e_temp != NULL) {
        edge *prox = e_temp->next;
        delete e_temp;
        e_temp = prox;
      }
    }
    
    delete[] G;
    G = nullptr;
  }
}