#include "decoders/mcp_decoder_threshould.hpp"

#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <iomanip>

using namespace std;
using namespace BRKGA;

//-----------------------------[ Constructor ]--------------------------------//

MCP_Decoder::MCP_Decoder(const MCP_Instance& _instance):
    instance(_instance)
{}

//-------------------------------[ Decode ]-----------------------------------//

BRKGA::fitness_t MCP_Decoder::decode(Chromosome& chromosome, bool /* not-used */)
{  
    /// Cost of the solution that the chromosome encodes
    double cost = 0;

    /// Threshold for edge removal
    double threshould = 0.5;

    /// Stores the acumulated sum of the cost of removed edges
    unsigned sum_cost_removed_edges = 0;

    /// Store the removed edges by chromossomo
    std::vector<bool> removed;
    
    /// Store the removed edges by chromossomo
    removed.resize(chromosome.size());
    
    /// Initialize removed edges vector
    for (unsigned i = 0; i < removed.size(); i++) {
        removed[i] = false;
    }

    /// Auxiliary variables
    unsigned v, edge_index;
    
    /// Traverses the graph and marks the removed edges
    for (unsigned u = 1; u <= instance.num_nodes; u++) {
        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++) {
            v = u_list[i].first;
            edge_index = instance.map_edge(u,v);

            if (chromosome[edge_index] > threshould) {
                removed[edge_index] = true;
                sum_cost_removed_edges += u_list[i].second; // edge cost
            }
        }
    }

    /// Compute fit 
    cost = (instance.num_terminals - bfs_separate_treminals(removed))
        + ((double)sum_cost_removed_edges / (double)instance.cumulative_edge_cost);

    return cost;
}


// BRKGA::fitness_t MCP_Decoder::decode(Chromosome& chromosome, bool /* not-used */)
// {
//     double cost = 0;
    
//     for(unsigned i = 0; i < instance.num_nodes; i++)
//         cost += i * chromosome[i];
    
//     return cost;
// }

//-----------------------------[ BFS_ST ]--------------------------------//

unsigned MCP_Decoder::bfs_separate_treminals(std::vector<bool>& removed) {

    /// Queue of nodes found
    std::queue<unsigned> queue;

    /// Partition of terminals in relation to connected components
    std::vector<std::vector<unsigned>> terminals_partition;

    /// Marker of nodes found
    std::vector<bool> visited(instance.num_nodes + 1);
    
    for (unsigned i = 1; i <= instance.num_nodes; i++)
        visited[i] = false;

    ////////////////////////////////////////
    // Breadth-first search to check
    // terminals that are separated
    ////////////////////////////////////////

    for (unsigned s: instance.terminals) {

        if (visited[s] == false) {
            
            visited[s] = true;

            /// Store the nodes rechable from s
            std::vector<unsigned> partition;
            partition.push_back(s);

            queue.push(s);

            while (!queue.empty()) {

                unsigned u = queue.front();
                queue.pop();
                
                /// Get the list of edges leaving u
                const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

                for (unsigned i = 0; i < u_list.size(); i++){
                    unsigned v = u_list[i].first;

                    if (visited[v] == false && removed[instance.map_edge(u,v)] == false) {
                        visited[v] = true;
                        queue.push(v);

                        if (std::find(instance.terminals.begin(), instance.terminals.end(), v) == instance.terminals.end())
                            partition.push_back(v);
                    }
                }
            }

            terminals_partition.push_back(partition);
        }
    }

    return terminals_partition.size();
}
