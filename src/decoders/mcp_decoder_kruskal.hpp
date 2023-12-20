#ifndef MCP_DECODER_KRUSKAL_HPP_
#define MCP_DECODER_KRUSKAL_HPP_

#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "brkga_mp_ipr/chromosome.hpp"


class MCP_Decoder_Kruskal {
public:
    /// A reference to a TSP instance.
    const MCP_Instance& instance;

    /// Auxiliary edge structure
    typedef struct {unsigned src; unsigned dst; unsigned cost;} edge;

    /// 
    std::vector<edge> list_edges;
    
public:
    /// \param instance MCP instance.
    MCP_Decoder_Kruskal(const MCP_Instance& instance);

    /// \param chromosome A vector of doubles represent a problem solution.
    /// \param rewrite Indicates if the chromosome must be rewritten. Not used
    ///                this decoder, but keep due to API requirements.
    /// \return the cost of the tour.
    BRKGA::fitness_t decode(BRKGA::Chromosome& chromosome, bool rewrite);

    void union_elements(std::vector<unsigned>& component, std::vector<int>& terminal, std::vector<unsigned>& rank, unsigned u, unsigned v);

    unsigned find_element(std::vector<unsigned>& component, unsigned v);

    void quick_sort(std::vector<double>& chromosome, std::vector<edge> &edges, int start, int end);

    int separa(std::vector<double>& chromosome, std::vector<edge> &edges, int start, int end);

    void troca_d(std::vector<double> &v, int a, int b);
    void troca_e(std::vector<edge> &edges, int a, int b); 
};

#endif // MCP_DECODER_HPP_
