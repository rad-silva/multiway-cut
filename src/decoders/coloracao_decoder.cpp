#include "decoders/coloracao_decoder.hpp"

#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace std;
using namespace BRKGA;

//-----------------------------[ Constructor ]--------------------------------//

/**
 * "In the instance G, there is an undirected graph with n nodes and m edges.
 * However, in our graph representation, it is necessary for G to actually 
 * have 2m arcs. That is, it requires an indicator that j is reachable from i 
 * and also an indicator that i is reachable from j.
 * 
 * Given that we have a chromosome of size m, with each position representing 
 * an arc, we want to map each of the 2m arcs to a position in this chromosome. 
 * To achieve this, we 'serialize' the adjacency lists, creating two auxiliary 
 * vectors: v1 of size 2m and v2 of size n.
 * 
 * - v1 will store all adjacency lists in sequence (first for node 1, then for 
 *   node 2, ...), and each position corresponding to an arc will map to its 
 *   respective index in the chromosome.
 * 
 * - v2 will store the position in v1 where the beginning of each node's 
 *   adjacency list is located.
 * 
 * Thus, to query the chromosome position corresponding to an arc (i, j), it is 
 * sufficient to apply to v1 the start of the adjacency list for i (v2[i]) added 
 * to the position of j in the adjacency list of i.
 **/

MCP_Decoder_Multiple_Thresholds::MCP_Decoder_Multiple_Thresholds(const MCP_Instance& _instance):
    instance(_instance),
    init_adjacency_list(instance.num_nodes + 1, -1),
    position_edge_vector(instance.num_edges * 2, -1)
{
    unsigned list_position = 0;
    unsigned edge_position = 0;
    unsigned u, v;
    
    for (u = 1; u <= instance.num_nodes; u++) {
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

BRKGA::fitness_t MCP_Decoder_Multiple_Thresholds::decode(Chromosome& chromosome, bool /* not-used */)
{  
    /// Stores the group that each node belongs to
    std::vector<int> group(instance.num_nodes + 1, -1);
    
    /// Compute group for non-terminal nodes
    for (unsigned u = 1; u <= instance.num_nodes; u++) {
        group[u] = std::floor(chromosome[u] * instance.num_terminals);
    }

    /// Compute group for terminal nodes
    for (unsigned i = 0; i < instance.num_terminals; i++) {
        unsigned s = instance.terminals[i];
        group[s] = i;
    }

    return bfs_group_treminals(group);
}

//-----------------------------[ BFS_GT ]--------------------------------//

unsigned MCP_Decoder_Multiple_Thresholds::bfs_group_treminals(std::vector<int>& group)
{
    /// Set of cuts
    std::vector<std::vector<edge>> set_cuts;

    /// Number of cuts an edge is part of
    std::vector<unsigned> num_cuts_edge(instance.num_edges, 0);

    /// Queue of nodes found
    std::queue<unsigned> queue;

    /// Marker of nodes found
    std::vector<bool> visited(instance.num_nodes + 1, false);
    
    /////////////////////////////////////////////////
    // Breadth-first search to find the nodes
    // of the same group reachable from the terminal
    /////////////////////////////////////////////////

    unsigned edge_index = 0;

    for (unsigned s: instance.terminals) {
            
        visited[s] = true;
        queue.push(s);

        /// Store the nodes rechable from s using
        /// nodes that are part of the same group
        std::vector<edge> set_edges_out_group;

        while (!queue.empty()) {

            unsigned u = queue.front();
            queue.pop();
            
            /// Get the list of edges leaving u
            const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

            for (unsigned i = 0; i < u_list.size(); i++){
                unsigned v = u_list[i].dst;

                if (group[v] == group[u]) {
                    if (visited[v] == false) {
                        visited[v] = true;
                        queue.push(v);
                    }
                }
                else {
                    /// Adds the arc to the set
                    set_edges_out_group.push_back({u, v, u_list[i].cost, i});

                    /// Compute the edge index
                    edge_index = position_edge_vector[init_adjacency_list[u] + i];
                    
                    /// Increases the number of cuts this edge participates in
                    num_cuts_edge[edge_index]++;
                }
            }
        }

        /// Adds edge set in cuts set
        set_cuts.push_back(set_edges_out_group);
    }
    
    /////////////////////////////////////////////
    // Calculates the actual cost of all cuts
    // minus the cost of the highest value cut
    /////////////////////////////////////////////

    /// Marking value for edges already considered in a cut
    unsigned IGNORE = -1;

    /// Total cost of cuts
    unsigned total_cost_cuts = 0;

    for (unsigned i = 0; i < set_cuts.size(); i++) {
        
        const std::vector<edge>& cut_i = set_cuts[i];

        for (edge e: cut_i) {
            edge_index = position_edge_vector[init_adjacency_list[e.src] + e.edge_index];

            if (num_cuts_edge[edge_index] == 2) {
                total_cost_cuts += e.cost;
                num_cuts_edge[edge_index] = IGNORE; // non-contabilize reverse edge
            }
        }
    }

    /// Highest cost cut (using bows exclusive to this cut)
    unsigned highest_cost_cut = 0;

    for (unsigned i = 0; i < set_cuts.size(); i++) {
        
        /// Get the i cut
        const std::vector<edge>& cut_i = set_cuts[i];

        /// Store the cost of cut i 
        unsigned edges_single_cut_cost = 0;

        /// Compute the actual cost of the cut:
        /// only the edges that are present in a single cut
        for (edge e: cut_i) {
            edge_index = position_edge_vector[init_adjacency_list[e.src] + e.edge_index];

            if (num_cuts_edge[edge_index] == 1)
                edges_single_cut_cost += e.cost;
        }

        /// Update the highest cost cut
        if (edges_single_cut_cost > highest_cost_cut) {
            highest_cost_cut = edges_single_cut_cost;
        }

        /// Update the total cust for all cuts
        total_cost_cuts += edges_single_cut_cost;
    }

    /// Discards the most costly cut
    return total_cost_cuts - highest_cost_cut;
}
