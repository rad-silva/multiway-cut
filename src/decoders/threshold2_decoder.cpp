#include "decoders/threshold2_decoder.hpp"

#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <iomanip>

using namespace std;
using namespace BRKGA;

//-----------------------------[ Constructor ]--------------------------------//

MCP_Decoder_Threshold2::MCP_Decoder_Threshold2(const MCP_Instance &_instance) : instance(_instance),
                                                                                init_adjacency_list(instance.num_nodes + 1, -1),
                                                                                position_edge_vector(instance.num_edges * 2, -1)
{
    unsigned list_position = 0;
    unsigned edge_position = 0;
    unsigned u, v;

    for (u = 1; u <= instance.num_nodes; u++)
    {
        init_adjacency_list[u] = list_position;

        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++)
        {
            v = u_list[i].dst;

            if (u < v)
            {
                position_edge_vector[list_position + i] = edge_position;
                edge_position++;
            }
            else
            { // A posição de (u,v) no vetor já foi determinada previamente a partir de (v,u)
                position_edge_vector[list_position + i] =
                    position_edge_vector[init_adjacency_list[v] + instance.get_edge_index(v, u)];
            }
        }

        list_position += u_list.size();
    }
}

//-------------------------------[ Decode ]-----------------------------------//

BRKGA::fitness_t MCP_Decoder_Threshold2::decode(Chromosome &chromosome, bool /* not-used */)
{
    /// Threshold Para remoção de arestas
    double threshold = 0.5;

    /// Armazena as arestas removidas pelo cromossomo
    std::vector<bool> removed(chromosome.size(), false);

    /// Variáveis auxiliar
    unsigned edge_index, v;

    /// Percorre o grafo e marca as arestas removidas
    for (unsigned u = 1; u <= instance.num_nodes; u++)
    {
        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++)
        {
            v = u_list[i].dst;

            if (u < v)
            {
                edge_index = position_edge_vector[init_adjacency_list[u] + i];

                if (chromosome[edge_index] > threshold)
                {
                    removed[edge_index] = true;
                }
            }
        }
    }

    return bfs_separate_treminals(removed);;
}

//-----------------------------[ BFS_ST ]--------------------------------//

double MCP_Decoder_Threshold2::bfs_separate_treminals(std::vector<bool> &removed)
{
    /// Armazena o numero de terminais que estão separados
    unsigned separated_terminals = instance.num_terminals;

    /// Armazena o número de cortes que um arco participa
    std::vector<unsigned> num_cuts_edge(instance.num_edges, 0);

    /// Marcador de nós visitados
    std::vector<bool> visited(instance.num_nodes + 1, false);

    /// fila de nós encontrados
    std::queue<unsigned> queue;

    /// Set of cuts
    std::vector<std::vector<edge>> set_edges_border_cuts;

    ///
    unsigned edge_index = 0;

    /// Armazena o custo total dos cortes
    unsigned total_cost_cuts = 0;

    ////////////////////////////////////////
    // Breadth-first search to check
    // terminals that are separated
    ////////////////////////////////////////

    for (unsigned s : instance.terminals)
    {
        // cout << "corte " << s << endl;

        /// Armazena os arcos que estão na fronteira da componente de s
        std::vector<edge> set_edges_border_component;

        if (visited[s] == false)
        {

            visited[s] = true;

            queue.push(s);

            while (!queue.empty())
            {

                unsigned u = queue.front();
                queue.pop();

                /// Get the list of edges leaving u
                const std::vector<MCP_Instance::edge> &u_list = instance.G[u];

                for (unsigned i = 0; i < u_list.size(); i++)
                {
                    unsigned v = u_list[i].dst;
                    edge_index = position_edge_vector[init_adjacency_list[u] + i];

                    if (removed[edge_index] == false)
                    {
                        if (visited[v] == false)
                        {
                            visited[v] = true;
                            queue.push(v);
                        }
                        // else
                            // v já foi visitado antes
                    }
                    else
                    { // removed[edge_index] == true
                        /// Incrementa o número de cortes que esta aresta participa
                        num_cuts_edge[edge_index]++;

                        /// Considera esse arco uma única vez no custo do corte
                        if (num_cuts_edge[edge_index] == 1)
                        {
                            total_cost_cuts += u_list[i].cost;
                        }

                        /// Adiciona o arco no conjunto
                        set_edges_border_component.push_back({u, v, u_list[i].cost, i});
                    }
                }
            }
        }
        else
        {
            separated_terminals -= 1;
        }

        /// Adiciona os arcos que fazem parte da fronteira da componente de s
        /// no vetor de conjuntos desses arcos para cada terminal
        set_edges_border_cuts.push_back(set_edges_border_component);
    }

    // Se existe algum terminal que não foi separado, nenhuma otimização na contabilização será feita
    if (separated_terminals < instance.num_terminals)
    {
        return (instance.num_terminals - separated_terminals) + ((double)total_cost_cuts / (double)instance.cumulative_edge_cost);
    }

    /////////////////////////////////////////////
    // Calcula o corte que de
    // maior custo bebenfício
    /////////////////////////////////////////////

    /// Armazena o valor do corte de maior custo (considerando arestas exclusivas a este corte)
    unsigned highest_cut_cost_exclusive_edges = 0;

    for (const std::vector<edge> &edges_border_cut : set_edges_border_cuts)
    {

        /// Armazena o custo das arestas exclusivas
        unsigned cost_exclusive_edges = 0;

        /// Calcula o custo acumulado do conjunto de arestas: apenas para arestas que aparecem somente na sua fronteira
        for (edge e : edges_border_cut)
        {
            edge_index = position_edge_vector[init_adjacency_list[e.src] + e.edge_index];

            if (num_cuts_edge[edge_index] == 1)
                cost_exclusive_edges += e.cost;
        }

        /// Atualiza o valor do corte de maior custo
        if (cost_exclusive_edges > highest_cut_cost_exclusive_edges)
        {
            highest_cut_cost_exclusive_edges = cost_exclusive_edges;
        }
    }

    return (double)(total_cost_cuts - highest_cut_cost_exclusive_edges) / (double)instance.cumulative_edge_cost;
}