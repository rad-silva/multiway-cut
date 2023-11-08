#include "mcp_instance.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iomanip>
#include <cmath>

using namespace std;

//-----------------------------[ Constructor ]--------------------------------//

MCP_Instance::MCP_Instance(const std::string& filename):
    num_nodes(0),
    num_edges(0),
    num_terminals(0),
    cumulative_edge_cost(0),
    G(),
    terminals()
{
    ifstream file(filename, ios::in);
    if(!file)
        throw runtime_error("Cannot open instance file");

    file.exceptions(ifstream::failbit | ifstream::badbit);
    
    unsigned u, v, cost;

    try {
        ////////////////////////////////////////
        // Reading the graph structure
        ////////////////////////////////////////

        file >> num_nodes >> num_edges; // número de arcos real do grafo é (pode ser) diferente

        G.resize(num_nodes + 1);

        for(unsigned i = 0; i < num_edges; i++) {
            file >> u >> v >> cost;

            // add edge in u list
            G[u].push_back(make_pair(v, cost));

            // add edge in v list
            G[v].push_back(make_pair(u, cost));

            // increment acumulative cost
            cumulative_edge_cost += cost;
        }

        ////////////////////////////////////////
        // Reading of terminal nodes
        ////////////////////////////////////////

        file >> num_terminals;

        terminals.resize(num_terminals);

        for(unsigned i = 0; i < num_terminals; i++) {
            file >> terminals[i];
        }
    }
    catch(std::ifstream::failure& e) {
        throw fstream::failure("Error reading the instance file");
    }
}

//-------------------------------[ Show ]---------------------------------//

void MCP_Instance::show()
{
    cout
    << std::setw(6) << "src"
    << std::setw(6) << "dest"
    << std::setw(6) << "cost" << endl;

    for(unsigned u = 0; u < num_nodes; u++) {
        std::vector<edge> u_list = G.at(u);
        
        for(unsigned e = 0; e < u_list.size(); e++) {
            cout
            << std::setw(6) << u 
            << std::setw(6) << u_list.at(e).first
            << std::setw(6) << u_list.at(e).second << endl;
        }
    }
}

//-----------------------------[ MAP EDGE ]--------------------------------//

unsigned MCP_Instance::map_edge(unsigned i, unsigned j) const
{
    if (i > j)
        std::swap(i,j);

    return ((i * ((num_nodes+1) - 1)) - ((i - 1) * i / 2) + (j - i - 1));
}

/*
n = 8
map_edge(1,2): 1 * (8 - 1) - (1 - 1) * 1 / 2 + (2 - 1 - 1) = 7
map_edge(1,3) 
:
*/

//-----------------------------[ UNMAP EDGE ]--------------------------------//

// unsigned MCP_Instance::unmap_index(unsigned p)
// {
//    return -1;
// }