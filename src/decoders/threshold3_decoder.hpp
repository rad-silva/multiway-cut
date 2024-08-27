#ifndef MCP_DECODER_THRESHOLD3_HPP_
#define MCP_DECODER_THRESHOLD3_HPP_

#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "brkga_mp_ipr/chromosome.hpp"


class MCP_Decoder_Threshold3 {
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
    MCP_Decoder_Threshold3(const MCP_Instance& instance);

    BRKGA::fitness_t decode(BRKGA::Chromosome& chromosome, bool rewrite);
};

#endif // MCP_DECODER_HPP_
