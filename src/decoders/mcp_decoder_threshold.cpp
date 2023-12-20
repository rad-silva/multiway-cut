#include "decoders/mcp_decoder_threshold.hpp"

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
    init_adjacency_list(instance.num_nodes + 1, -1),
    position_edge_vector(instance.num_edges * 2, -1)
{
    /// \brief Go through the adjoining list of each node in the order
    /// u = 1,...n. For an edge (u,v), if u > v, the arc (v,u) has 
    /// already been analyzed, so the position that (u,v) will be mapped
    /// to in the vector must be the same as that defined for (v,u).
    /// In the opposite case, the arc (u,v) is defined normally,
    /// discounting the cases where two equal edges have already occurred.

    unsigned index = 0, discount = 0, edge_list_position, v;
    
    for (unsigned u = 1; u <= instance.num_nodes; u++) {
        init_adjacency_list[u] = index;

        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++) {
            v = u_list[i].dst;

            if (u > v) {
                edge_list_position = get_edge_index(v,u);
                position_edge_vector[index + i] = position_edge_vector[init_adjacency_list[v] + edge_list_position];
                discount++;
            }
            else {
                edge_list_position = i;
                position_edge_vector[index + i] = index + edge_list_position - discount;
            }
        }

        index += instance.G[u].size();
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
    unsigned edge_index;
    
    /// Traverses the graph and marks the removed edges
    for (unsigned u = 1; u <= instance.num_nodes; u++) {
        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++) {
            edge_index = position_edge_vector[init_adjacency_list[u] + i];

            if (chromosome[edge_index] > threshold && u < u_list[i].dst) {
                removed[edge_index] = true;
                sum_cost_removed_edges += u_list[i].cost; // edge cost
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
    std::vector<bool> visited(instance.num_nodes + 1, false);

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

//-----------------------------[ Get edge index ]--------------------------------//

unsigned MCP_Decoder_Single_Threshold::get_edge_index(unsigned u, unsigned v)
{
    const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

    for (unsigned i = 0; i < u_list.size(); i++){
        if (v == u_list[i].dst)
            return i;
    }

    return 0;
}