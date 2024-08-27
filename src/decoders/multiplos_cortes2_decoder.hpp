#ifndef MCP_DECODER_CUTS2_HPP_
#define MCP_DECODER_CUTS2_HPP_

#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "brkga_mp_ipr/chromosome.hpp"


class MCP_Decoder_Cuts2 {
public:
    /// A reference to a TSP instance.
    const MCP_Instance& instance;

    /// Position of the beginning of the adjacency list of each node in vector
    std::vector<unsigned> init_adjacency_list;

    /// Position of each edge in the edge vector (mapping 2*m -> m)
    std::vector<unsigned> position_edge_vector;

public:
    /// Constructor and main function of the decoder
    MCP_Decoder_Cuts2(const MCP_Instance& instance);

    BRKGA::fitness_t decode(BRKGA::Chromosome& chromosome, bool rewrite);
};

#endif // MCP_DECODER_HPP_
