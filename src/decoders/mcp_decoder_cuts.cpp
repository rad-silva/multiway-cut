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

BRKGA::fitness_t MCP_Decoder_Cuts::decode(Chromosome &chromosome, bool /* not-used */)
{
    double accumulated_cost_cuts = 0;

    //////////////////////////////////////////////////
    // creates a residual graph instance with the
    // costs of the arcs perturbed by the chromosome
    //////////////////////////////////////////////////

    unsigned u, v, cost; 
    unsigned edge_index;
    MCP_Instance::edge edge_e;

    // the n+1 vertex will be used as an auxiliary node to perform the cuts
    Graph G(instance.num_nodes + 1);

    for (u = 1; u <= instance.num_nodes; u++)
    {
        for (unsigned i = 0; i < instance.G[u].size(); i++)
        {
            edge_e = instance.G[u][i];
            v = edge_e.dst;
            cost = edge_e.cost;

            if (u < v) {
                edge_index = position_edge_vector[init_adjacency_list[u] + i];
                G.add_edge_with_reverse(u, v, (cost * chromosome[edge_index]), i);
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
                solver.add_edge(sj, t, std::numeric_limits<double>::max(), 0);
            }
        }

        // Run algorithm that calculates the maximum flow
        solver.solve(si, t);

        cout << "cut " << si << ": " << solver.get_max_flow() << endl; 

        // Store de edges cut
        cuts.push_back(solver.get_edges_cut());
    }

    //////////////////////////////////////////////////
    // Calculates the number of times 
    // each arc appeared in a cut
    //////////////////////////////////////////////////

    /// Number of cuts an edge is part of
    std::vector<unsigned> num_cuts_edge(instance.num_edges, 0);

    for (unsigned i = 0; i < cuts.size(); i++)
    {
        for (highest_push_relabel_max_flow::edge e : cuts[i])
        {
            /// Compute the edge index
            edge_index = position_edge_vector[init_adjacency_list[e.src] + e.index];

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
            /// Compute the edge index
            edge_index = position_edge_vector[init_adjacency_list[e.src] + e.index];

            if (num_cuts_edge[edge_index] == 1)
                cost_cut += instance.G[e.src][e.index].cost;
            if (num_cuts_edge[edge_index] == 2) {
                accumulated_cost_cuts += instance.G[e.src][e.index].cost;
                num_cuts_edge[edge_index] = -1;
            }
        }

        accumulated_cost_cuts += cost_cut;

        if (cost_cut > highest_cost)
        {
            highest_cost = cost_cut;
        }
    }

    cout << accumulated_cost_cuts - highest_cost << endl;
    cout << "-----------------------------------|\n\n";
    return accumulated_cost_cuts - highest_cost;
}