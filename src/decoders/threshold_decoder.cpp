#include "decoders/threshold_decoder.hpp"

#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <iomanip>

using namespace std;
using namespace BRKGA;

//-----------------------------[ Constructor ]--------------------------------//

MCP_Decoder_Single_Threshold::MCP_Decoder_Single_Threshold(const MCP_Instance& _instance):
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
            else { // The position of (u,v) in the vector has already been previously determined from (v,u)
                position_edge_vector[list_position + i] = 
                    position_edge_vector[init_adjacency_list[v] + instance.get_edge_index(v,u)];
            }
        }

        list_position += u_list.size();
    }
}

//-------------------------------[ Decode ]-----------------------------------//

BRKGA::fitness_t MCP_Decoder_Single_Threshold::decode(Chromosome& chromosome, bool /* not-used */)
{ 
    /// Cost of the solution that the chromosome encodes
    double cost = 0;

    /// Threshold for edge removal
    double threshold = 0.5;

    /// Stores the acumulated sum of the cost of removed edges
    unsigned sum_cost_removed_edges = 0;

    /// Store the removed edges by chromossomo
    std::vector<bool> removed(chromosome.size(), false);

    /// Auxiliary variable
    unsigned edge_index, v;
    
    /// Traverses the graph and marks the removed edges
    for (unsigned u = 0; u < instance.num_nodes; u++) {
        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++) {
            v = u_list[i].dst;

            if (u < v) {
                edge_index = position_edge_vector[init_adjacency_list[u] + i];

                if (chromosome[edge_index] > threshold) {
                    removed[edge_index] = true;
                    sum_cost_removed_edges += u_list[i].cost; // add edge to the cut
                }
            }
        }
    }

    /// Compute fit 
    cost = (instance.num_terminals - bfs_separate_treminals(removed))
        + ((double)sum_cost_removed_edges / (double)instance.cumulative_edge_cost);

    return cost;
}

//-----------------------------[ BFS_ST ]--------------------------------//

unsigned MCP_Decoder_Single_Threshold::bfs_separate_treminals(std::vector<bool>& removed) {

    unsigned separated_terminals = instance.num_terminals;

    /// Marker of nodes found
    std::vector<bool> visited(instance.num_nodes, false);

    /// Queue of nodes found
    std::queue<unsigned> queue;

    unsigned edge_index = 0;
    
    ////////////////////////////////////////
    // Breadth-first search to check
    // terminals that are separated
    ////////////////////////////////////////

    for (unsigned s: instance.terminals) {

        if (visited[s] == false) {
            
            visited[s] = true;

            queue.push(s);

            while (!queue.empty()) {

                unsigned u = queue.front();
                queue.pop();
                
                /// Get the list of edges leaving u
                const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

                for (unsigned i = 0; i < u_list.size(); i++){
                    unsigned v = u_list[i].dst;
                    edge_index = position_edge_vector[init_adjacency_list[u] + i];

                    if (visited[v] == false && removed[edge_index] == false) {
                        visited[v] = true;
                        queue.push(v);
                    }
                }
            }
        } 
        else {
            separated_terminals -= 1;
        }
    }

    return separated_terminals;
}