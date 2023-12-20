#include "decoders/mcp_decoder_cuts.hpp"
#include "max_flow/graph.hpp"
#include "max_flow/highest_push_relabel.hpp"

#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace std;
using namespace BRKGA;

//-----------------------------[ Constructor ]--------------------------------//

MCP_Decoder_Cuts::MCP_Decoder_Cuts(const MCP_Instance &_instance) : 
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

    for (unsigned u = 1; u <= instance.num_nodes; u++)
    {
        init_adjascency_list[u] = index;

        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++)
        {
            v = u_list[i].dst;

            if (u > v)
            {
                edge_list_position = instance.get_edge_index(v, u);
                position_edge_vector[index + i] = position_edge_vector[init_adjascency_list[v] + edge_list_position];
                discount++;
            }
            else
            {
                edge_list_position = i;
                position_edge_vector[index + i] = index + edge_list_position - discount;
            }
        }

        index += instance.G[u].size();
    }
}

//-------------------------------[ Decode ]-----------------------------------//

BRKGA::fitness_t MCP_Decoder_Cuts::decode(Chromosome &chromosome, bool /* not-used */)
{
    double accumulated_cost_cuts = 0;

    //////////////////////////////////////////////////
    // creates a residual graph instance with the
    // costs of the arcs perturbed by the chromosome
    //////////////////////////////////////////////////

    unsigned u, v, cost, edge_index = 0;
    MCP_Instance::edge e;

    // the n+1 vertex will be used as an auxiliary node to perform the cuts
    Graph G(instance.num_nodes + 1);

    for (u = 1; u <= instance.num_nodes; u++)
    {
        for (unsigned i = 0; i < instance.G[u].size(); i++)
        {
            e = instance.G[u][i];
            v = e.dst;
            cost = e.cost;

            if (u < v)
            {
                G.add_edge_with_reverse(u, v, (cost * chromosome[edge_index]));
                edge_index++;
            }
        }
    }

    //////////////////////////////////////////////////
    // calculates the cuts that isolate
    // each si terminal from the others
    //////////////////////////////////////////////////

    unsigned t = instance.num_nodes + 1;

    vector<vector<highest_push_relabel_max_flow::edge>> cuts;

    for (unsigned si : instance.terminals)
    {
        // Create hpr object
        highest_push_relabel_max_flow solver(G);

        for (unsigned sj : instance.terminals)
        {
            if (sj != si)
            {
                solver.add_edge(sj, t, std::numeric_limits<double>::max());
            }
        }

        // Run algorithm that calculates the maximum flow
        solver.solve(si, t);

        // Store de edges cut
        cuts.push_back(solver.get_edges_cut());
    }

    //////////////////////////////////////////////////
    // Calculates the number of times 
    // each arc appeared in a cut
    //////////////////////////////////////////////////

    /// Number of cuts an edge is part of
    std::vector<unsigned> num_cuts_edge(instance.num_edges, 0);
    unsigned edge_list_position;

    for (unsigned i = 0; i < cuts.size(); i++)
    {
        for (highest_push_relabel_max_flow::edge e : cuts[i])
        {
            edge_list_position = instance.get_edge_index(e.src, e.dst);

            /// Compute the edge index
            edge_index = position_edge_vector[init_adjascency_list[e.src] + edge_list_position];


            /// Increases the number of cuts this edge participates in
            num_cuts_edge[edge_index]++;
        }
    }

    //////////////////////////////////////////////////
    // Calculates the cost of all cuts.
    // It also calculates the highest real cost cut (only
    // considering arcs that are part of a single cut)
    //////////////////////////////////////////////////

    unsigned highest_cost = 0;

    for (unsigned i = 0; i < cuts.size(); i++)
    {
        double cost_cut = 0;

        for (highest_push_relabel_max_flow::edge e : cuts[i])
        {
            edge_list_position = instance.get_edge_index(e.src, e.dst);

            /// Compute the edge index
            edge_index = position_edge_vector[init_adjascency_list[e.src] + edge_list_position];

            if (num_cuts_edge[edge_index] == 1)
                cost_cut += instance.G[e.src][edge_list_position].cost;
            if (num_cuts_edge[edge_index] == 2 and e.src < e.dst)
                accumulated_cost_cuts += instance.G[e.src][edge_list_position].cost;
        }

        accumulated_cost_cuts += cost_cut;

        if (cost_cut > highest_cost)
        {
            highest_cost = cost_cut;
        }
    }

    return accumulated_cost_cuts - highest_cost;
}