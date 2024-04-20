#ifndef MCP_DECODER_SINGLE_THRESHOLD_HPP_
#define MCP_DECODER_SINGLE_THRESHOLD_HPP_

#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "brkga_mp_ipr/chromosome.hpp"


class MCP_Decoder_Single_Threshold {
public:
    /// A reference to a MCP instance.
    const MCP_Instance& instance;

    /// Position of the beginning of the adjacency list of each node in vector
    std::vector<unsigned> init_adjacency_list;

    /// Position of each edge in the edge vector (mapping 2*m -> m)
    std::vector<unsigned> position_edge_vector;

public:
    /// Constructor and main function of the decoder
    MCP_Decoder_Single_Threshold(const MCP_Instance& instance);

    BRKGA::fitness_t decode(BRKGA::Chromosome& chromosome, bool rewrite);

    /// Search edges in cut
    unsigned bfs_separate_treminals(std::vector<bool>& removed);
};

#endif // MCP_DECODER_HPP_
