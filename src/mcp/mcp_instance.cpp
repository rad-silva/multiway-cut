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

MCP_Instance::MCP_Instance(const std::string &filename, unsigned type_file) : num_nodes(0),
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
    unsigned u, v, cost;
    unsigned s, t;

    try
    {
        if (type_file == 1)
        {
            ////////////////////////////////////////
            // Reading the graph structure
            ////////////////////////////////////////

            file >> num_nodes >> num_edges; // número de arcos real do grafo é (pode ser) diferente

            G.resize(num_nodes + 1);

            for (unsigned i = 0; i < num_edges; i++)
            {
                file >> u >> v >> cost;

                // add edge in u list
                G[u].push_back({v, cost});

                // add edge in v list
                G[v].push_back({u, cost});

                // increment acumulative cost
                cumulative_edge_cost += cost;
            }

            ////////////////////////////////////////
            // Reading of terminal nodes
            ////////////////////////////////////////

            file >> num_terminals;

            terminals.resize(num_terminals);

            for (unsigned i = 0; i < num_terminals; i++)
            {
                file >> terminals[i];
            }
        }
        if (type_file == 2)
        {
            ////////////////////////////////////////
            // Reading the graph structure
            ////////////////////////////////////////

            // // read line 1: <mark> <tipo_problema> <num_nodes> <num_edges>
            file >> mark >> discard >> num_nodes >> num_edges;

            G.resize(num_nodes + 1);

            // read line 2: <mark> <rotule_node> <type_src>
            // read line 3: <mark> <rotule_node> <type_sink>
            file >> mark >> s >> discard;
            file >> mark >> t >> discard;

            num_terminals = 2;
            terminals.push_back(s);
            terminals.push_back(t);

            int edge_index;

            // reads the lines of the arcs:
            // <mark> <src_node> <dst_node> <cost_edge>
            // ignore empty lines and lines with other markers
            while (file >> mark)
            {
                if (mark == "a")
                {
                    file >> u >> v >> cost;
                    edge_index = get_edge_index(u, v);

                    if (edge_index == -1)
                    {
                        // add edge in u list
                        G[u].push_back({v, cost});

                        // add edge in v list
                        G[v].push_back({u, cost});
                    }
                    else
                    {
                        G[u][edge_index].cost = cost;
                    }
                    // increment acumulative cost
                    cumulative_edge_cost += cost;

                    cout << u << " " << v << " " << cost << endl;
                }
                else
                {
                    getline(file, discard);
                }
            }
        }
    }
    catch (std::ifstream::failure &e)
    {
        throw fstream::failure("Error reading the instance file");
    }
}

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
}