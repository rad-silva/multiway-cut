#include "decoders/multiplos_cortes2_decoder.hpp"
#include "max_flow/graph.hpp"
#include "max_flow/highest_push_relabel.hpp"

#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace std;
using namespace BRKGA;

// unsigned NUM_CROMOSOMO = 0;

// int geracao = 0;

//-----------------------------[ Constructor ]--------------------------------//

MCP_Decoder_Cuts2::MCP_Decoder_Cuts2(const MCP_Instance &_instance) : instance(_instance),
                                                                    init_adjacency_list(instance.num_nodes, -1),
                                                                    position_edge_vector(instance.num_edges * 2, -1)
{
    unsigned list_position = 0;
    unsigned edge_position = 0;
    unsigned u, v;

    for (u = 0; u < instance.num_nodes; u++)
    {
        init_adjacency_list[u] = list_position;

        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++)
        {
            v = u_list[i].dst;

            if (u < v)
            {
                position_edge_vector[list_position + i] = edge_position;
                edge_position++;
            }
            else
            { // The position of (u,v) in the vector has already been previously determined from (v,u)
                position_edge_vector[list_position + i] =
                    position_edge_vector[init_adjacency_list[v] + instance.get_edge_index(v, u)];
            }
        }

        list_position += u_list.size();
    }
}

//-------------------------------[ Decode ]-----------------------------------//

BRKGA::fitness_t MCP_Decoder_Cuts2::decode(Chromosome &chromosome, bool /* not-used */)
{
    // cout << NUM_CROMOSOMO << ": ";
    // for (auto x: chromosome)
    //     cout << x << " ";
    // cout << endl << endl;
    // NUM_CROMOSOMO++;

    //////////////////////////////////////////////////
    // Cria um grafo auxiliar com os custos dos
    // arcos perturbados pelo cromossomo
    //////////////////////////////////////////////////

    double perturbed_capacity;
    int N = 10;

    unsigned u, v, cost;
    unsigned edge_index;
    MCP_Instance::edge edge_e;

    // No grafo original os rótulos dos nós vão de 0 a n-1.
    // No grafo auxiliar (grafo residual para o algoritmo de fluxo máximo)
    // os rótulos dos nós vão de 0 a n, onde o nó de rótulo n será usado como
    // um sorvedouro comum a todos os terminais exceto o que estamos tentando separar, que é a fonte.
    Graph G(instance.num_nodes);

    for (u = 0; u < instance.num_nodes; u++)
    {
        for (unsigned i = 0; i < instance.G[u].size(); i++)
        {
            edge_e = instance.G[u][i];
            v = edge_e.dst;
            cost = edge_e.cost;

            if (u < v)
            {
                edge_index = position_edge_vector[init_adjacency_list[u] + i];

                // [2^(N/2) * custo(e)] / [2^(N*(1-chromosome(e)))]
                perturbed_capacity = (double)(pow(2, N / 2) * cost) / (double)(pow(2, (N * (1 - chromosome[edge_index]))));
                G.add_edge_with_reverse(u, v, perturbed_capacity, i);
            }
        }
    }

    //////////////////////////////////////////////////
    // Calcula os cortes que separam 
    // cada terminal si dos demais
    //////////////////////////////////////////////////

    unsigned t = instance.num_nodes;

    vector<vector<highest_push_relabel_max_flow::edge>> cuts;

    for (unsigned si : instance.terminals)
    {
        highest_push_relabel_max_flow solver(G);

        // adiciona um arco de custo infinito entre todos os terminais sj e t (sj != si)
        for (unsigned sj : instance.terminals)
        {
            if (sj != si)
                solver.add_edge(sj, t, std::numeric_limits<double>::max(), 0);
        }

        // Executa o algoritmo que calcula o fulxo máximo e corte mínimo entre si e t
        solver.solve(si, t);

        // Armazena os arcos que fazem parte do corte
        cuts.push_back(solver.get_edges_cut());
    }

    //////////////////////////////////////////////////
    // Marca as arestas que foram cortadas
    //////////////////////////////////////////////////

    std::vector<bool> cuted_edge(instance.num_edges, false);

    for (unsigned i = 0; i < cuts.size(); i++)
    {
        for (highest_push_relabel_max_flow::edge e : cuts[i])
        {
            /// Computa o indice da aresta no vetor
            edge_index = position_edge_vector[init_adjacency_list[e.src] + e.index];
            cuted_edge[edge_index] = true;
        }
    }

    //////////////////////////////////////////////////
    // Realiza uma busca (no grafo original) para identificar 
    // a partição que cada nó pertence (existe uma partição por terminal).
    // Cada partição recebe o rótulo do terminal associado a ela
    // Juntamente, computa o custo do multiway corte
    //////////////////////////////////////////////////

    /// Armazena o número de vezes que um arco apareceu na fronteira de um corte
    std::vector<unsigned> num_cuts_edge(instance.num_edges, 0);

    /// Marcador de nós visitados
    std::vector<int> visited_in_first_search(instance.num_nodes, -1);

    /// Fila de nós encontrados
    std::queue<unsigned> queue;

    /// Armazena o custo da solução
    double cost_of_multiway_cut = 0;


    for (unsigned terminal_index = 0; terminal_index < instance.num_terminals; terminal_index++)
    {

        unsigned s = instance.terminals[terminal_index];
        queue.push(s);

        while (!queue.empty())
        {

            u = queue.front();
            queue.pop();

            visited_in_first_search[u] = terminal_index;

            // Obtém a lista de adjacência de u
            const std::vector<MCP_Instance::edge> &u_list = instance.G[u];

            for (unsigned i = 0; i < u_list.size(); i++)
            {

                v = u_list[i].dst;
                edge_index = position_edge_vector[init_adjacency_list[u] + i];

                if (cuted_edge[edge_index] == true) // aresta removida
                {
                    /// Incrementa o número de fronteiras que o arco foi encontrado
                    num_cuts_edge[edge_index]++;

                    /// Considera o custo deste arco apenas uma vez
                    if (num_cuts_edge[edge_index] == 1) {
                        cost_of_multiway_cut += u_list[i].cost;
                    }
                }
                else // cuted_edge[edge_index] == false // aresta não removida
                {
                    if (visited_in_first_search[v] == -1) 
                    {
                        queue.push(v);
                    }
                    // else
                        // visited_in_first_search[v] != -1 // v já foi visitado
                }
            }
        }
    }

    //////////////////////////////////////////////////
    // Realiza uma busca a partir de cada nó isolado
    // (nós em que visited_in_first_search[v] == -1)
    // para atribuí-los à partição que mais reduzirá
    // o custo da solução final
    //////////////////////////////////////////////////

    std::vector<int> visited_in_second_search(instance.num_nodes, -1);

    for (unsigned w = 0; w < instance.num_nodes; w++) {

        if (visited_in_first_search[w] == -1 && visited_in_second_search[w] == -1) {

            queue.push(w);

            /// Armazena o custo das arestas na fronteira da componente formada 
            /// por essa busca separado por cada rótulo de partição encontrada
            std::vector<unsigned> cost_edges_border_by_partition(instance.num_terminals, 0);

            typedef struct {unsigned edge_index;} aux_edge;

            std::vector<std::vector<aux_edge>> edges_indexes(instance.num_terminals);

            while (!queue.empty()) {

                /// Remove o primeiro nó da fila de encontrados
                u = queue.front();
                queue.pop();

                if (visited_in_second_search[u] == 1) {
                    continue; // esse nó já foi visitado
                }

                visited_in_second_search[u] = 1;
                
                /// Obtém a lista de arestas que saem de u
                const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

                for (unsigned i = 0; i < u_list.size(); i++) {

                    v = u_list[i].dst;
                    cost = u_list[i].cost;

                    if (visited_in_first_search[v] == -1) { // v é um nó isolado
                        if (visited_in_second_search[v] == -1) { // v não foi visitado nessa busca
                            queue.push(v);
                        }
                        // else
                            // visited_in_second_search[v] != -1 // v já foi visitado nessa busca
                    }
                    else { // visited_in_first_search[v] != -1 // v não é um nó isolado
                        cost_edges_border_by_partition[visited_in_first_search[v]] += cost;
                        edges_indexes[visited_in_first_search[v]].push_back({position_edge_vector[init_adjacency_list[u] + i]});
                    }
                }
            }

            double best_border_cost = 0;
            int terminal_atribuido = -1;

            for (unsigned partition = 0; partition < instance.num_terminals; partition++) {
                if (cost_edges_border_by_partition[partition] > best_border_cost) {
                    best_border_cost = cost_edges_border_by_partition[partition];
                    terminal_atribuido = partition;
                }
            }

            cost_of_multiway_cut -= best_border_cost;


            for (aux_edge x: edges_indexes[terminal_atribuido]) {
                num_cuts_edge[x.edge_index] = 0;
            }
        }
    }

    typedef struct {unsigned src; unsigned dst; unsigned cost; unsigned edge_index;} aux_edge;
    cout << endl;
    cout << "p " << "edge " << instance.num_nodes << " " << instance.num_edges << endl;
    cout << "c " << cost_of_multiway_cut << endl;

    for (unsigned u = 0; u < instance.num_nodes; u++) {

        const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++)
        {
            v = u_list[i].dst;
            edge_index = position_edge_vector[init_adjacency_list[u] + i];
            cost = u_list[i].cost;

            if (num_cuts_edge[edge_index] > 0) {
                cout << "e " << u << " " << v << " " << cost << " " << edge_index << endl;
                num_cuts_edge[edge_index] = 0;
            }
        }
    }

    exit(1);

    
    return cost_of_multiway_cut;
}
