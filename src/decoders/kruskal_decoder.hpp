#ifndef MCP_DECODER_KRUSKAL_HPP_
#define MCP_DECODER_KRUSKAL_HPP_

#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "brkga_mp_ipr/chromosome.hpp"


class MCP_Decoder_Kruskal {
public:
    /// A reference to a MCP instance.
    const MCP_Instance& instance;

    /// Auxiliary edge structure
    typedef struct {unsigned src; unsigned dst; unsigned cost;} edge;

    /// Auxiliary structure to store the edges of Graph
    std::vector<edge> list_edges;
    
public:
    /// Constructor and main function of the decoder
    
    MCP_Decoder_Kruskal(const MCP_Instance& instance);
  
    BRKGA::fitness_t decode(BRKGA::Chromosome& chromosome, bool rewrite);

    /// Custom union-find data structure functions

    void union_elements(
        std::vector<unsigned>& component, 
        std::vector<int>& terminal, 
        std::vector<unsigned>& rank, 
        unsigned u, unsigned v
    );

    unsigned find_element(
        std::vector<unsigned>& component, 
        unsigned v
    );

    /// Quick Sort functions

    void quick_sort(
        std::vector<double>& chromosome, 
        std::vector<edge> &edges, 
        int start, int end
    );

    int separa(
        std::vector<double>& chromosome, 
        std::vector<edge> &edges,
        int start, int end
    );

    void troca_d(std::vector<double> &v, int a, int b);
    void troca_e(std::vector<edge> &edges, int a, int b); 
};

#endif // MCP_DECODER_HPP_
