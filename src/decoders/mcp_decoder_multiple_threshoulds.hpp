#ifndef MCP_DECODER_MULTIPLE_THRESHOULDS_HPP_
#define MCP_DECODER_MULTIPLE_THRESHOULDS_HPP_

#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "brkga_mp_ipr/chromosome.hpp"

/*
 * TB_Decoder: Threshold Based Decoder
 * 
 */

class MCP_Decoder_Multiple_Threshoulds {
public:
    /// A reference to a TSP instance.
    const MCP_Instance& instance;

public:
    /// \param instance MCP instance.
    MCP_Decoder_Multiple_Threshoulds(const MCP_Instance& instance);

    /// \param chromosome A vector of doubles represent a problem solution.
    /// \param rewrite Indicates if the chromosome must be rewritten. Not used
    ///                this decoder, but keep due to API requirements.
    /// \return the cost of the tour.
    BRKGA::fitness_t decode(BRKGA::Chromosome& chromosome, bool rewrite);

    unsigned bfs_group_treminals(std::vector<int>& removed);
};

#endif // MCP_DECODER_HPP_
