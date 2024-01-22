#include "decoders/mcp_decoder_kruskal_pert.hpp"

#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace std;
using namespace BRKGA;

//-----------------------------[ Constructor ]--------------------------------//

MCP_Decoder_Kruskal_Pertubation::MCP_Decoder_Kruskal_Pertubation(const MCP_Instance &_instance) :
    instance(_instance),
    list_edges(instance.num_edges)
{
    unsigned u, v, cost;
    unsigned edge_position = 0;

    /// Inserts each edge into a list
    for (u = 1; u <= instance.num_nodes; u++)
    {
        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++)
        {
            v = u_list[i].dst;
            cost = u_list[i].cost;

            if (u < v)
            { // does not insert reverse arcs
                list_edges[edge_position] = {u, v, cost};
                edge_position++;
            }
        }
    }
}

//-------------------------------[ Decode ]-----------------------------------//

BRKGA::fitness_t MCP_Decoder_Kruskal_Pertubation::decode(Chromosome &chromosome, bool /* not-used */)
{
    /// edges cuted
    double cost_cut = 0;

    /// Copy vector of edges
    vector<edge> edges = list_edges;

    /// Auxiliar vector for pertuberd cost of edges
    vector<double> perturbed_costs(instance.num_edges);

    /// Structures for union-find
    std::vector<unsigned> component(instance.num_nodes + 1, 0);
    std::vector<int> terminal(instance.num_nodes + 1, -1);
    std::vector<unsigned> rank(instance.num_nodes + 1, 0);

    /// Mark who the component is and the terminal that each node is part of
    for (unsigned v = 1; v <= instance.num_nodes; v++) {
        component[v] = v;
        terminal[v] = -1;
        rank[v] = 0;
    }

    for (unsigned s : instance.terminals) {
        terminal[s] = s;
    }

    /// Upsets the cost of edges
    for (unsigned i = 0; i < instance.num_edges; i++) {
        perturbed_costs[i] = (edges[i].cost * chromosome[i]) * -1;
    }

    /// Order the chromosomesome based on the value of the alleles.
    /// Modifies the position of the edges together
    quick_sort(perturbed_costs, edges, 0, chromosome.size()-1);

    /// Auxiliary variables
    unsigned u, v, cost;
    unsigned cu, cv;

    /// Traverses the edge vector by adding the edges that
    /// do not join components with different terminals.
    /// In the opposite case, the edge is added to the cut.
    for (unsigned i = 0; i < instance.num_edges; i++)
    {
        u = edges[i].src;
        v = edges[i].dst;
        cost = edges[i].cost;

        cu = find_element(component, u);
        cv = find_element(component, v);

        if (cu != cv)
        {
            if (terminal[cu] == -1 || terminal[cv] == -1)
                union_elements(component, terminal, rank, u, v);
            else
                // uses the original cost
                cost_cut += cost;
        }
    }

    return cost_cut;
}

//-----------------------------[ Union ]--------------------------------//

void MCP_Decoder_Kruskal_Pertubation::union_elements(std::vector<unsigned>& component, std::vector<int>& terminal, std::vector<unsigned>& rank, unsigned u, unsigned v)
{
    unsigned c1 = find_element(component, u);
    unsigned c2 = find_element(component, v);

    if (rank[c1] == rank[c2])
        rank[c1]++;

    if (rank[c1] > rank[c2])
    {
        component[c2] = c1;
        if (terminal[c1] == -1)
            terminal[c1] = terminal[c2];
    }
    else
    {
        component[c1] = c2;
        if (terminal[c2] == -1)
            terminal[c2] = terminal[c1];
    }
}

//-----------------------------[ Find ]--------------------------------//

unsigned MCP_Decoder_Kruskal_Pertubation::find_element(std::vector<unsigned>& component, unsigned v)
{
    if (component[v] == v)
        return v;

    component[v] = find_element(component, component[v]);

    return component[v];
}

//-----------------------------[ Quick sort ]--------------------------------//

void MCP_Decoder_Kruskal_Pertubation::troca_d(std::vector<double>&v, int a, int b)
{
    double aux = v[a];
    v[a] = v[b];
    v[b] = aux;
}

void MCP_Decoder_Kruskal_Pertubation::troca_e(std::vector<edge>& edges, int a, int b)
{
    edge aux = edges[a];
    edges[a] = edges[b];
    edges[b] = aux;
}

// separa v[p .. r] e devolve a posição do pivô
int MCP_Decoder_Kruskal_Pertubation::separa(std::vector<double>& v, std::vector<edge> &edges, int p, int r)
{
    int i, j;
    double c = v[r]; // c é o pivô
    i = p;
    for (j = p; j < r; j++)
        if (v[j] <= c)
        {
            troca_d(v, i, j);
            troca_e(edges, i, j);
            i++;
        }
    troca_d(v, i, r);
    troca_e(edges, i, r);
    return i;
}

// p indica a primeira posicao e r a ultima
void MCP_Decoder_Kruskal_Pertubation::quick_sort(std::vector<double>& v, std::vector<edge> &edges, int p, int r)
{
    int i;
    if (p < r) // se vetor corrente tem mais de um elemento
    {
        i = separa(v, edges, p, r); // i é a posição do pivô após a separação
        quick_sort(v, edges, p, i - 1);
        quick_sort(v, edges, i + 1, r);
    }
}