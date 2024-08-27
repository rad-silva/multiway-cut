#include "decoders/threshold3_decoder.hpp"

#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <iomanip>

using namespace std;
using namespace BRKGA;

//-----------------------------[ Constructor ]--------------------------------//

MCP_Decoder_Threshold3::MCP_Decoder_Threshold3(const MCP_Instance &_instance) : 
    instance(_instance),
    init_adjacency_list(instance.num_nodes, -1),
    position_edge_vector(instance.num_edges * 2, -1)
{
    unsigned list_position = 0;
    unsigned edge_position = 0;
    unsigned u, v;

    for (u = 0; u < instance.num_nodes; u++)
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

BRKGA::fitness_t MCP_Decoder_Threshold3::decode(Chromosome &chromosome, bool /* not-used */)
{
    /// Threshold para remoção de arestas
    double threshold = 0.5;

    /// Armazena o número de terminais que estão separados
    unsigned separated_terminals = instance.num_terminals;

    /// Armazena o número de cortes que um aresta participa
    std::vector<unsigned> num_cuts_edge(instance.num_edges, 0);

    /// Marcador de nós visitados
    std::vector<unsigned> visited(instance.num_nodes, -1);

    /// Fila de nós encontrados
    std::queue<unsigned> queue;

    /// Armazena os conjuntos de arestas dos cortes (um conjunto/corte para cada terminal)
    std::vector<std::vector<edge>> set_edges_border_cuts;

    /// Auxilia na identificação da posição de uma aresta <e> em um vetor de tamanho <m>
    unsigned edge_index = 0;


    ////////////////////////////////////////
    // Busca em largura para econtrar
    // a fronteira de cada corte
    ////////////////////////////////////////

    for (unsigned s : instance.terminals)
    {

        /// Armazena os arcos que estão na fronteira da componente de s
        std::vector<edge> set_edges_border_component;

        // visited[s] = s;

        queue.push(s);

        while (!queue.empty())
        {

            unsigned u = queue.front();
            queue.pop();

            visited[u] = s;

            /// Obtém a lista de adjacência de u
            const std::vector<MCP_Instance::edge> &u_list = instance.G[u];

            for (unsigned i = 0; i < u_list.size(); i++)
            {

                unsigned v = u_list[i].dst;
                edge_index = position_edge_vector[init_adjacency_list[u] + i];

                if (chromosome[edge_index] < threshold) // arco não marcado como removido pelo cromossomo
                {
                    if (visited[v] == -1) // v nunca foi encontrado em uma busca
                    {
                        auto it = std::find(instance.terminals.begin(), instance.terminals.end(), v);

                        if (it == instance.terminals.end()) // v não é um terminal
                        {
                            // visited[v] = s;
                            queue.push(v);
                        }
                        else // v é um terminal
                        {
                            // força esse arco a ser marcado como removido
                            chromosome[edge_index] = 0.75;

                            /// Incrementa o número de cortes que esta aresta participa
                            num_cuts_edge[edge_index]++;

                            /// Adiciona o arco no conjunto
                            set_edges_border_component.push_back({u, v, u_list[i].cost, i});
                        }

                        
                    }
                    // else
                        // v já foi visitado antes
                }
                else // arco marcado como removido
                {
                    if (visited[v] != s) // v nunca foi encontrado em uma busca ou foi visitado por uma busca a partir de outro terminal
                    {
                        /// Incrementa o número de cortes que esta aresta participa
                        num_cuts_edge[edge_index]++;

                        /// Adiciona o arco no conjunto
                        set_edges_border_component.push_back({u, v, u_list[i].cost, i});
                    }
                    else // v já foi encontrado pela busca deste mesmo terminal
                    {
                        // essa aresta deixa de ser considerado como removida
                        num_cuts_edge[edge_index] = 0;
                        // chromosome[edge_index] = 0.25;
                    }
                }
            }
        }

        /// Adiciona os arcos que fazem parte da fronteira da componente de s
        /// no vetor de conjuntos desses arcos para cada terminal
        set_edges_border_cuts.push_back(set_edges_border_component);
    }

    /////////////////////////////////////////////
    // Calcula o corte que de
    // maior custo bebenfício
    /////////////////////////////////////////////

    /// Armazena o valor do corte de maior custo (considerando arestas exclusivas a este corte)
    unsigned cost_multiway_cut = 0;
    unsigned highest_cost_cut_using_exclusive_edges = 0;

    for (const std::vector<edge> &edges_border_cut : set_edges_border_cuts)
    {

        /// Armazena o custo das arestas exclusivas para o corte atual
        unsigned cost_cut_using_exclusive_edges = 0;

        /// Calcula o custo acumulado do conjunto de arestas: apenas para arestas que aparecem somente na sua fronteira
        for (edge e : edges_border_cut)
        {
            edge_index = position_edge_vector[init_adjacency_list[e.src] + e.edge_index];

            if (num_cuts_edge[edge_index] == 1) {
                cost_cut_using_exclusive_edges += e.cost;
            }

            if (num_cuts_edge[edge_index] > 0) {
                cost_multiway_cut += e.cost;
                num_cuts_edge[edge_index] = 0; // evita que essa aresta seja contabilizada novamente (para o caso num_cuts_edge[edge_index] == 2)
            }
        }

        /// Atualiza o valor do corte de maior custo
        if (cost_cut_using_exclusive_edges > highest_cost_cut_using_exclusive_edges)
            highest_cost_cut_using_exclusive_edges = cost_cut_using_exclusive_edges;
    }

    return cost_multiway_cut - highest_cost_cut_using_exclusive_edges;
}
