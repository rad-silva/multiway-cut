#include "decoders/coloracao2_decoder.hpp"

#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace std;
using namespace BRKGA;

/*
    Adicionando 
*/

//-----------------------------[ Constructor ]--------------------------------//

MCP_Decoder_Coloracao2::MCP_Decoder_Coloracao2(const MCP_Instance& _instance):
    instance(_instance),
    init_adjacency_list(instance.num_nodes, -1),
    position_edge_vector(instance.num_edges * 2, -1)
{
    unsigned list_position = 0;
    unsigned edge_position = 0;
    unsigned u, v;
    
    for (u = 0; u < instance.num_nodes; u++) {
        init_adjacency_list[u] = list_position;

        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++) {
            v = u_list[i].dst;

            if (u < v) {
                position_edge_vector[list_position + i] = edge_position;
                edge_position++;
            }
            else { // A posição de (u,v) no vetor já foi determinada previamente a partir de (v,u)
                position_edge_vector[list_position + i] = 
                    position_edge_vector[init_adjacency_list[v] + instance.get_edge_index(v,u)];
            }
        }

        list_position += u_list.size();
    }
}

//-------------------------------[ Decode ]-----------------------------------//

BRKGA::fitness_t MCP_Decoder_Coloracao2::decode(Chromosome& chromosome, bool /* not-used */)
{  
    /// Armazena a cor de cada nó
    std::vector<int> color(instance.num_nodes, -1);

    /// Marcador de nós que originalmente podem ser visitados a partir de um terminal
    std::vector<bool> originally_visited_node(instance.num_nodes, false);
    
    /// Determina a cor dos nós não-terminal
    for (unsigned u = 0; u < instance.num_nodes; u++) {
        color[u] = std::floor(chromosome[u] * instance.num_terminals);
    }

    /// Determina a cor de cada dos nós terminal
    for (unsigned i = 0; i < instance.num_terminals; i++) {
        unsigned s = instance.terminals[i];
        color[s] = i;
    }

    // realiza a marcação dos nós que originalmente podem ser visitados a partir de um terminal
    bfs_get_original_visited_nodes(color, originally_visited_node);

    // recolore nós isolados e calcula valor do corte
    return bfs_recolor_nodes(color, originally_visited_node, chromosome);
}

//-----------------------------[ Calcula nós que podem ser visitados ]--------------------------------//

void MCP_Decoder_Coloracao2::bfs_get_original_visited_nodes(
    std::vector<int>& color,
    std::vector<bool>& originally_visited_node)
{
    /// Fila que armazena os nós encontrados
    std::queue<unsigned> queue;

    /// Marcador de nós visitados
    std::vector<bool> visited(instance.num_nodes, false);
    
    /////////////////////////////////////////////////
    // Breadth-first search para encontrar os nós de
    // mesma cor que são alcançáveis a partir de cada
    // terminal
    /////////////////////////////////////////////////

    unsigned edge_index = 0;

    for (unsigned s: instance.terminals) {
            
        visited[s] = true;
        queue.push(s);

        while (!queue.empty()) {

            unsigned u = queue.front();
            queue.pop();
            
            /// Obtem a lista de arestas que saem de u
            const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

            for (unsigned i = 0; i < u_list.size(); i++){
                unsigned v = u_list[i].dst;

                if (color[v] == color[u]) {
                    if (visited[v] == false) {
                        visited[v] = true;
                        queue.push(v);
                    }
                }
            }
        }
    }

    originally_visited_node = visited;
}

//-----------------------------[ Recolore nós ]--------------------------------//

unsigned MCP_Decoder_Coloracao2::bfs_recolor_nodes(
    std::vector<int>& color, 
    std::vector<bool>& originally_visited_node, 
    BRKGA::Chromosome& chromosome
) {
    /// Armazena o número de cortes que um arco participa
    std::vector<unsigned> num_cuts_edge(instance.num_edges, 0);

    /// Fila  que armazena os nós encontrados
    std::queue<unsigned> queue;

    /// Marcador de nós visitados
    std::vector<bool> visited(instance.num_nodes, false);

    // Armazena o índice de uma aresta na lista de adjascência
    unsigned edge_index = 0;

    // Armazena o custo dos arcos que atravessam todos os cortes
    unsigned total_cost_cuts = 0;
    
    /////////////////////////////////////////////////
    // Breadth-first search para encontrar e recolorir
    // nós que não puderam ser originalmente visitados
    /////////////////////////////////////////////////

    for (unsigned s: instance.terminals) {
            
        visited[s] = true;
        queue.push(s);

        // Armazena o custo dos arcos que atravessam o corte que inclui o terminal s
        unsigned cost_cut_of_s = 0;

        while (!queue.empty()) {

            unsigned u = queue.front();
            queue.pop();
            
            /// Obtem a lista de arestas que saem de u
            const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

            for (unsigned i = 0; i < u_list.size(); i++){
                unsigned v = u_list[i].dst;

                if (color[v] == color[u]) {
                    if (visited[v] == false) {
                        visited[v] = true;
                        queue.push(v);
                    }                  
                }
                else {
                    if (originally_visited_node[v] == false) {
                        /* v é um nó isolado e pode ser atribuído à componente de u */

                        visited[v] = true;
                        queue.push(v);
                        
                        // Altera a cor de v
                        color[v] = color[u];

                        // Converte o alelo de v no cromossomo
                        chromosome[v] = ((double)color[v] / (double)instance.num_terminals) + ((double)1 / (2*instance.num_terminals));
                    }
                    else {
                        /* v ainda vai ser visitado pelo temrinal de sua cor.
                        Portanto, O arco (u,v) é uma fronteira entre as componentes de 
                        dois terimais distintos então deve ser contabilizado no corte */

                        /// Calcula o índice do arco
                        edge_index = position_edge_vector[init_adjacency_list[u] + i];
                        
                        /// Incrementa o número de cortes que esta aresta participa
                        num_cuts_edge[edge_index]++;

                        /// Considera esse arco uma única vez no custo do corte
                        if (num_cuts_edge[edge_index] == 1)
                            // TODO: colocar o arco no conjuto de arcos removidos para salvar no arquivo de saída
                            cost_cut_of_s += u_list[i].cost;
                    }
                }
            }
        }

        total_cost_cuts += cost_cut_of_s;
    }
    
    return total_cost_cuts;
}












/*
/// Armazena os arcos que atravessam o corte do terminal s
/// (arcos que ligam a nós que são encontrados por ele,
/// mas não pode ser visitados por terem cor diferente da sua)
std::vector<edge> set_edges_out_component;


/// Armazena as cores e o número total de terminais que encontraram cada nó
    typedef struct terminals_found {
        std::vector<unsigned> colors;
        unsigned total = 0;
    };

    std::vector<terminals_found> found(instance.num_nodes);

for (unsigned s: instance.terminals) {
            
        visited[s] = true;
        queue.push(s);

        /// Armazena os arcos que atravessam o corte do terminal s
        /// (arcos que ligam a nós que são encontrados por ele,
        /// mas não pode ser visitados por terem cor diferente da sua)
        std::vector<edge> set_edges_out_group;

        while (!queue.empty()) {

            unsigned u = queue.front();
            queue.pop();
            
            /// Obtem a lista de arestas que saem de u
            const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

            for (unsigned i = 0; i < u_list.size(); i++){
                unsigned v = u_list[i].dst;

                if (color[v] == color[u]) {
                    if (visited[v] == false) {
                        visited[v] = true;
                        queue.push(v);
                    }
                }
                else {
                    // Indica se o conjunto da cor de u já encotrou o nó v antes
                    bool color_marker = false;

                    for (unsigned found_color : found[v].colors) {
                        if (found_color == color[u]) {
                            color_marker = true;
                            break;
                        }
                    }

                    if (color_marker == false) {
                        found[v].colors.push_back(color[u]);
                        found[v].total++;
                    }

                    // /// Adiciona o arco no conjunto
                    // set_edges_out_group.push_back({u, v, u_list[i].cost, i});

                    // /// Calcula o índice do arco
                    // edge_index = position_edge_vector[init_adjacency_list[u] + i];
                    
                    // /// Aumenta o número de cortes dos quais esta aresta participa
                    // num_cuts_edge[edge_index]++;
                }
            }
        }

        /// Adiciona o conjunto de arestas no conjunto de cortes
        set_cuts.push_back(set_edges_out_group);
    }
*/