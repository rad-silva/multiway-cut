#ifndef MCP_DECODER_COLORACAO2_HPP_
#define MCP_DECODER_COLORACAO2_HPP_

#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "brkga_mp_ipr/chromosome.hpp"


class MCP_Decoder_Coloracao2 {
public:
    /// Auxiliary edge structure
    typedef struct {unsigned src; unsigned dst; unsigned cost; unsigned edge_index; } edge;

    /// A reference to a MCP instance.
    const MCP_Instance& instance;

    /// Position of the beginning of the adjacency list of each node in vector
    std::vector<unsigned> init_adjacency_list;

    /// Position of each edge in the edge vector (mapping 2*m -> m)
    std::vector<unsigned> position_edge_vector;

public:
    /// Constructor and main function of the decoder

    MCP_Decoder_Coloracao2(const MCP_Instance& instance);

    BRKGA::fitness_t decode(BRKGA::Chromosome& chromosome, bool rewrite);

    /// Search edges in cut

    void bfs_get_original_visited_nodes(std::vector<int>& color, std::vector<bool>& originally_visited_node);

    unsigned bfs_recolor_nodes(std::vector<int>& color, std::vector<bool>& originally_visited_node, BRKGA::Chromosome& chromosome);

    unsigned get_edge_index(unsigned u, unsigned v);
};

#endif // MCP_DECODER_HPP_
