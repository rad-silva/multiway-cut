#ifndef MCP_DECODER_THRESHOLD2_HPP_
#define MCP_DECODER_THRESHOLD2_HPP_

#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "brkga_mp_ipr/chromosome.hpp"


class MCP_Decoder_Threshold2 {
public:
    /// Uma referência de MCP instance.
    const MCP_Instance& instance;

    /// Posição do início da lista de adjacência de cada nó no vetor
    std::vector<unsigned> init_adjacency_list;

    /// Posição de cada aresta no vetor de arestas (mapeaia 2*m -> m)
    std::vector<unsigned> position_edge_vector;

    /// Auxiliary edge structure
    typedef struct {unsigned src; unsigned dst; unsigned cost; unsigned edge_index; } edge;

public:
    /// Constructor and main function of the decoder
    MCP_Decoder_Threshold2(const MCP_Instance& instance);

    BRKGA::fitness_t decode(BRKGA::Chromosome& chromosome, bool rewrite);

    /// Search edges in cut
    double bfs_separate_treminals(std::vector<bool>& removed);
};

#endif // MCP_DECODER_HPP_
