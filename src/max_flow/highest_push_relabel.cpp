#include <vector>
#include <queue>
#include <algorithm>
#include <chrono>

#include "graph.hpp"
#include "highest_push_relabel.hpp"
#include "priority_queue.hpp"
#include "list_node_height.hpp"

highest_push_relabel_max_flow::highest_push_relabel_max_flow(Graph &G) : 
  Gf(G),
  src(0), sink(0), num_nodes(Gf.num_vertices()), 
  node_excess(num_nodes+1, 0), node_height(num_nodes+1, 0),
  list_height(Gf.num_vertices()), bucket()
{ }


// Satura todos os arcos que saem da fonte no grafo Residual -> f(s,v) = u(s,v)
void highest_push_relabel_max_flow::initializePreflow()
{
  for (Graph::edge *e = Gf.get_edge(src); e != NULL; e = e->next) {
    if (e->capacity > 0) {
      pushFlow(src, e, e->capacity);
    }
  }

  node_height[src] = num_nodes;
  list_height.update_h(src, num_nodes);
}



// Empurra excesso contido no nó u para o nó v
// Note: flow deve ser igual a min(node_excess[u], e->capacity);
void highest_push_relabel_max_flow::pushFlow(int u, Graph::edge *e, double flow)
{
  // Atualiza o fluxo e capacidade dos arcos (u,v) e (v,u) no grafo residual
  e->flow += flow;
  e->reverse->flow -= flow;

  e->capacity -= flow;
  e->reverse->capacity += flow;

  // Insere o nó que recebeu excesso na fila se ele é diferente de s e t, e não possuia excesso antes da operação
  if (node_excess[e->destNode] == 0 and (e->destNode != src and e->destNode != sink)) {
    bucket.push(e->destNode, node_height[e->destNode]);
  }

  // Atualiza o excesso de u e de v
  node_excess[u] -= flow;
  node_excess[e->destNode] += flow; 

  num_pushs++;
}


// Se existir algum valor k < n tal que existam nós ativos i com rótulo de distância h(i) > k,
// mas nenhum nó j com rótulo de distância h(j) = k (includo os não ativos),
// então definimos h(i) = n para cada nó i com k < h(i) < n.
void highest_push_relabel_max_flow::gap(int k)
{
  if (k >= num_nodes) return;

  while (list_height.empty_h(k) == false)
  {
    int u = list_height.front_h(k);
    list_height.update_h(u, num_nodes);
    node_height[u] = num_nodes;
  }

  if (list_height.empty_h(k+1) == false)
    gap(k+1);
}


void highest_push_relabel_max_flow::relabel(int u)
{
  // backup da altura atual de u
  int h_u = node_height[u];

  int h_min = 2 * num_nodes;
  Graph::edge *e;

  for (e = Gf.get_edge(u); e != NULL; e = e->next) {
    if (e->capacity > 0) {
      if (node_height[e->destNode] < h_min) {
        h_min = node_height[e->destNode];
      }
    }
  }

  node_height[u] = h_min + 1;

  // Como no algoritmo highest label Push-Relabel pegamos sempre
  // o nó com maior node_height, d* sempre aumenta ao fazer relabel
  if (node_height[u] > d) {
    d = node_height[u];
  }

  // Verifica se existe um Gap nessa altura
  if (list_height.empty_h(h_u) == true) {
    gap(h_u+1);
    node_height[u] = num_nodes;
    num_gaps++;
  }

  num_relabels++;
}


bool highest_push_relabel_max_flow::isAdmissible(int u, Graph::edge *e)
{
  return (e->capacity > 0 and node_height[u] == node_height[e->destNode] + 1);
}


// Enquanto um nó u tiver excesso positivo realiza operações de push e de relabel
void highest_push_relabel_max_flow::discharge(int u)
{
  Graph::edge *e;
  list_height.remove_h(u);

  while (node_excess[u] > 0 and node_height[u] < num_nodes) {
    for (e = Gf.get_edge(u); node_excess[u] > 0 and e != nullptr; e = e->next) {
      if (isAdmissible(u, e)) {
        double delta = std::min(node_excess[u], e->capacity);
        pushFlow(u, e, delta);
      }
    }

    if (node_excess[u] > 0) {
      relabel(u);
    }
  }

  list_height.update_h(u, node_height[u]);
}


int highest_push_relabel_max_flow::solve(int s, int t)
{
  auto start = std::chrono::steady_clock::now();

  src = s;
  sink = t;
  d = 0;

  list_height.initialize_all_height(0);
  bucket.clear();

  initializePreflow();

  while (!bucket.is_empty()) {
    int u = bucket.pop();
    discharge(u);
  }

  max_flow = node_excess[t];

  auto end = std::chrono::steady_clock::now();
  time_execution = (end - start).count();

  return max_flow;
}

void highest_push_relabel_max_flow::print_local( ) {
  std::cout << "node_excess[";
  for (int u = 0; u <= num_nodes; u++) {
    std::cout << node_excess[u] << " ";
  } std::cout << "]\n";

  std::cout << "node_height[";
  for (int u = 0; u <= num_nodes; u++) {
    std::cout << node_height[u] << " ";
  } std::cout << "]\n";

  std::cout << "list_height\n";
  list_height.print();

  std::cout << "bucket\n";
  bucket.printQueue();

  std::cout << "-------------------------------\n";
}

void highest_push_relabel_max_flow::add_edge(int u, int v, double cost, unsigned index) {
  Gf.add_edge_with_reverse(u, v, cost, index);
}

std::vector<highest_push_relabel_max_flow::edge> highest_push_relabel_max_flow::get_edges_cut()
{
  /// Queue of nodes found
  std::queue<int> queue;

  /// Marker of nodes found
  std::vector<bool> visited(num_nodes + 1, false);
  
  /////////////////////////////////////////////////
  // Breadth-first search to find the nodes
  // of the same group reachable from the terminal
  /////////////////////////////////////////////////
 
  visited[src] = true;
  queue.push(src);

  while (!queue.empty()) {

    int u = queue.front();
    queue.pop();
      
    Graph::edge *e;

    for (e = Gf.get_edge(u); e != nullptr; e = e->next){
      int v = e->destNode;

      if (visited[v] == false && e->capacity > 0) {
        visited[v] = true;
        queue.push(v);
      }
    }
  }

  /////////////////////////////////////////////
  // 
  // 
  /////////////////////////////////////////////

  std::vector<edge> edges_cuted;

  for (int u = 0; u <= num_nodes; u++) {
    if (visited[u] == true) {
      Graph::edge *e;

      for (e = Gf.get_edge(u); e != nullptr; e = e->next){
        int v = e->destNode;

        if (visited[v] == 0) {
          if (u < v)
            edges_cuted.push_back(edge{u, v, e->index});
          else 
            edges_cuted.push_back(edge{v, u, e->index});
        }
      }
    }
  }

  return edges_cuted;
}
