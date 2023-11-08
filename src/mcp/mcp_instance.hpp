#ifndef MCP_INSTANCE_HPP_
#define MCP_INSTANCE_HPP_

#include <string>
#include <vector>
#include <utility>

/*
 * Representação de uma instancia do Multiway Cut Problem.
 * O construtor carrega um grafo no seguinte formato:
 * 
 * número de nós (n) número de arcos (m)
 * nó_i nó_j capacidade
 *          ...
 * nó_u nó_v capacidade
 * número de terminais (k)
 * s1 s2 ... sk
 */


class MCP_Instance {
public:
    /// Number of nodes.
    unsigned num_nodes;

    /// Number of edges.
    unsigned num_edges;

    /// Number of terminals.
    unsigned num_terminals;

    /// Accumulated sum of edges cost
    unsigned cumulative_edge_cost;

    /// Representation of edges: make_pair(destination_node, capacity)
    typedef std::pair<unsigned, unsigned> edge;

    /// Graph in adjascency list representation.
    std::vector<std::vector<edge>> G;

    /// Terminals S.
    std::vector<unsigned> terminals;

public:
    /// Default Constructor.
    MCP_Instance(const std::string& filename);

    /// Map edge
    /// \brief given the labels i and j, calculates the index p
    /// corresponding to the arc (i,j) for a vector of size n*(n-1)/2
    /// \param i node label
    /// \param j node label
    /// \return index 
    unsigned map_edge(unsigned i, unsigned j) const;

    /// Unmap index:
    /// \brief given index p of a vector, calculates the label of
    /// the nodes corresponding to the arc represented by this index.
    /// \param p node label
    /// \return i,j labes nodes
    // unsigned unmap_index(unsigned p);

    /// Show graph
    void show();
};

#endif // MCP_INSTANCE_HPP_
