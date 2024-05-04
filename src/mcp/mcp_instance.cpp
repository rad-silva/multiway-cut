#include "mcp_instance.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iomanip>
#include <cmath>
#include <bits/stdc++.h>


using namespace std;

//-----------------------------[ Constructor ]--------------------------------//

MCP_Instance::MCP_Instance(const std::string &filename) : 
    num_nodes(0),
    num_edges(0),
    num_terminals(0),
    cumulative_edge_cost(0),
    G(),
    terminals()
{
    ifstream file(filename, ios::in);
    if (!file)
    {
        throw runtime_error("Cannot open instance file");
    }

    string mark, discard;
    unsigned u, v;
    double cost;
    string line;

    try
    {
        ////////////////////////////////////////
        // Reading the graph structure
        ////////////////////////////////////////

        // Ignoring the first line
        getline(file, line);

        // Lendo rótulos dos terminais
        getline(file, line);

        stringstream lineStream(line);
        int terminal;

        while (lineStream >> terminal) {
            terminals.push_back(terminal);
        }

        // O primeiro elemento do vetor terminals é o número de terminais
        num_terminals = terminals[0];

        terminals.erase(terminals.begin());

        // Lendo numero de nós e arestas
        file >> mark >> discard >> num_nodes >> num_edges;

        G.resize(num_nodes + 1);

        for (unsigned i = 0; i < num_edges; i++)
        {
            file >> mark >> u >> v >> cost;

            // add edge in u list
            G[u].push_back({v, (unsigned int)cost});

            // add edge in v list
            G[v].push_back({u, (unsigned int)cost});

            // increment acumulative cost
            cumulative_edge_cost += cost;
        }

        show();
    
    }
    catch (std::ifstream::failure &e)
    {
        throw fstream::failure("Error reading the instance file");
    }
}

//--------------------------[ Get Edge Index ]----------------------------//

int MCP_Instance::get_edge_index(unsigned u, unsigned v) const
{
    vector<edge> u_list = G[u];

    for (unsigned edge_index = 0; edge_index < u_list.size(); edge_index++)
        if (u_list[edge_index].dst == v)
            return edge_index;

    return -1;
}

//-------------------------------[ Show ]---------------------------------//

void MCP_Instance::show() const
{
    cout
    << "Nodes: " << num_nodes << endl
    << "Edges: " << num_edges << endl << endl;

    cout
    << std::setw(6) << "src"
    << std::setw(6) << "dest"
    << std::setw(6) << "cost" << endl;

    for (unsigned u = 0; u <= num_nodes; u++)
    {
        std::vector<edge> u_list = G.at(u);

        for (unsigned e = 0; e < u_list.size(); e++)
        {
            cout
            << std::setw(6) << u
            << std::setw(6) << u_list.at(e).dst
            << std::setw(6) << u_list.at(e).cost << endl;
        }
    }
    cout << endl;

    cout << "Terminals: " << num_terminals << endl;
    
    cout << "[";
    for (unsigned i : terminals) { cout << i << " "; }
    cout << "]" << endl;
    
    cout << endl;
}



///////////////// Reader Maximum Flow Instances

// unsigned s, t;
        
// // read line 1: <mark> <tipo_problema> <num_nodes> <num_edges>
// file >> mark >> discard >> num_nodes >> num_edges;

// G.resize(num_nodes + 1);

// // read line 2: <mark> <rotule_node> <type_src>
// // read line 3: <mark> <rotule_node> <type_sink>
// file >> mark >> s >> discard;
// file >> mark >> t >> discard;

// num_terminals = 2;
// terminals.push_back(s);
// terminals.push_back(t);

// int edge_index;

// // reads the lines of the arcs:
// // <mark> <src_node> <dst_node> <cost_edge>
// // ignore empty lines and lines with other markers
// while (file >> mark)
// {
//     if (mark == "a")
//     {
//         file >> u >> v >> cost;
//         edge_index = get_edge_index(u, v);

//         if (edge_index == -1)
//         {
//             // add edge in u list
//             G[u].push_back({v, cost});

//             // add edge in v list
//             G[v].push_back({u, cost});

//             // increment acumulative cost
//             cumulative_edge_cost += cost;
//         }
//         else
//         {
//             G[u][edge_index].cost = cost;
//         }
//     }
//     else
//     {
//         getline(file, discard);
//     }
// }
// show();


///////////////// Reader Steiner Tree Instances

// string line;

// /// Find <SECTION Graph> marker
// while (getline(file, line) && line.find("SECTION Graph") == string::npos) { }

// file >> mark >> num_nodes;
// file >> mark >> num_edges; // número de arcos real do grafo é (pode ser) diferente

// G.resize(num_nodes + 1);

// for (unsigned i = 0; i < num_edges; i++)
// {
//     file >> mark >> u >> v >> cost;

//     // add edge in u list
//     G[u].push_back({v, cost});

//     // add edge in v list
//     G[v].push_back({u, cost});

//     // increment acumulative cost
//     cumulative_edge_cost += cost;
// }

// ////////////////////////////////////////
// // Reading of terminal nodes
// ////////////////////////////////////////

// // Find <SECTION Terminals> marker
// while (getline(file, line) && line.find("SECTION Terminals") == string::npos) { }

// file >> mark >> num_terminals;

// terminals.resize(num_terminals);

// for (unsigned i = 0; i < num_terminals; i++)
// {
//     file >> mark >> terminals[i];
// }