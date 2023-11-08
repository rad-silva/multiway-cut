#ifndef MCP_DECODER_LIMIAR_HPP_
#define MCP_DECODER_LIMIAR_HPP_

#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "brkga_mp_ipr/chromosome.hpp"

/*
 * TB_Decoder: Threshold Based Decoder
 * 
 */

class MCP_Decoder {
public:
    /// A reference to a TSP instance.
    const MCP_Instance& instance;

public:
    /// \param instance MCP instance.
    MCP_Decoder(const MCP_Instance& instance);

    /// \param chromosome A vector of doubles represent a problem solution.
    /// \param rewrite Indicates if the chromosome must be rewritten. Not used
    ///                this decoder, but keep due to API requirements.
    /// \return the cost of the tour.
    BRKGA::fitness_t decode(BRKGA::Chromosome& chromosome, bool rewrite);

    unsigned bfs_separate_treminals(std::vector<bool>& removed);
};

#endif // MCP_DECODER_HPP_
