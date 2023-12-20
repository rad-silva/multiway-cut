#include "decoders/mcp_decoder_multiple_thresholds.hpp"

#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace std;
using namespace BRKGA;

//-----------------------------[ Constructor ]--------------------------------//

MCP_Decoder_Multiple_Thresholds::MCP_Decoder_Multiple_Thresholds(const MCP_Instance& _instance):
    instance(_instance),
    init_adjascency_list(instance.num_nodes + 1, -1),
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
        init_adjascency_list[u] = index;

        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++) {
            v = u_list[i].dst;

            if (u > v) {
                edge_list_position = get_edge_index(v,u);
                position_edge_vector[index + i] = position_edge_vector[init_adjascency_list[v] + edge_list_position];
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

//-----------------------------[ Get edge index ]--------------------------------//

unsigned MCP_Decoder_Multiple_Thresholds::get_edge_index(unsigned u, unsigned v)
{
    const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

    for (unsigned i = 0; i < u_list.size(); i++){
        if (v == u_list[i].dst)
            return i;
    }

    return 0;
}

//-------------------------------[ Decode ]-----------------------------------//

BRKGA::fitness_t MCP_Decoder_Multiple_Thresholds::decode(Chromosome& chromosome, bool /* not-used */)
{  
    /// Stores the group that each node belongs to
    std::vector<int> group(instance.num_nodes + 1, -1);
    
    /// Compute group for non-terminal nodes
    for (unsigned i = 1; i <= instance.num_nodes; i++) {
        group[i] = std::floor(chromosome[i] * instance.num_terminals);
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
                    set_edges_out_group.push_back({u, v, u_list[i].cost});

                    /// Compute the edge index
                    edge_index = position_edge_vector[init_adjascency_list[u] + i];
                    
                    /// Increases the number of cuts this edge participates in
                    num_cuts_edge[edge_index]++;
                }
            }
        }

        set_cuts.push_back(set_edges_out_group);
    }

    /////////////////////////////////////////////
    // calculates the actual cost of all cuts
    // minus the cost of the highest value cut
    /////////////////////////////////////////////

    /// Marking value for edges already considered in a cut
    unsigned IGNORE = -1;

    /// Total cost of cuts
    unsigned total_cost_cuts = 0;

    for (unsigned i = 0; i < set_cuts.size(); i++) {
        
        const std::vector<edge>& cut_edges = set_cuts[i];

        for (edge e: cut_edges) {
            edge_index = position_edge_vector[init_adjascency_list[e.src] + get_edge_index(e.src, e.dst)];

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
        const std::vector<edge>& cut_edges = set_cuts[i];

        /// Store the cost of cut i 
        unsigned edges_single_cut_cost = 0;

        /// Compute the actual cost of the cut:
        /// only the edges that are present in a single cut
        for (edge e: cut_edges) {
            edge_index = position_edge_vector[init_adjascency_list[e.src] + get_edge_index(e.src, e.dst)];

            if (num_cuts_edge[edge_index] == 1)
                edges_single_cut_cost += e.cost;
        }

        /// Update the highest cost cut
        if (edges_single_cut_cost > highest_cost_cut) {
            highest_cost_cut = edges_single_cut_cost;
        }

        /// Update the total cust of cuts
        total_cost_cuts += edges_single_cut_cost;
    }

    /// Discards the most costly cut
    return total_cost_cuts - highest_cost_cut;
}
