#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr.hpp"
#include "brkga_mp_ipr/chromosome.hpp"

#include "write_file.hpp"
#include "execute_decoders.hpp"

#include "geradores/geradores_cromossomo.hpp"

#include "decoders/threshold_decoder.hpp"
#include "decoders/coloracao_decoder.hpp"
#include "decoders/kruskal_decoder.hpp"
#include "decoders/kruskal_pert_decoder.hpp"
#include "decoders/multiplos_cortes_decoder.hpp"
#include "decoders/multiplos_cortes2_decoder.hpp"
#include "decoders/coloracao2_decoder.hpp"
#include "decoders/threshold2_decoder.hpp"
#include "decoders/coloracao3_decoder.hpp"
#include "decoders/threshold3_decoder.hpp"

#include "max_flow/highest_push_relabel.hpp"
#include "max_flow/graph.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <queue>

using namespace std;

//-------------------------------[ Decoder execution ]------------------------------------//

void execute_threshold(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing Decoder <threshold>" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file);

        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] = BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {max_run_time};

        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;

        MCP_Decoder_Single_Threshold decoder(instance);

        unsigned chromossome_size = instance.num_edges;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Single_Threshold> algorithm (
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

        algorithm.reset(); // chama #initialize(true)

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time << "s..." << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness * instance.cumulative_edge_cost
        << endl;

        ////////////////////////////////////////
        // Save results in file
        ////////////////////////////////////////

        unsigned edge_index, v, cost;

        unsigned num_edges_cut = 0;

        vector<format_edge> edges_cuted;

        for (unsigned i = 0; i < final_status.best_chromosome.size(); i++) {
            if (final_status.best_chromosome[i] > 0.5) {
                num_edges_cut++;
            }
        }

        for (unsigned u = 0; u < instance.num_nodes; u++) {
            std::vector<MCP_Instance::edge> u_list = instance.G[u];

            for (unsigned i = 0; i < u_list.size(); i++) {
                edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[u] + i];
                v = u_list[i].dst;
                cost = u_list[i].cost;

                if (final_status.best_chromosome[edge_index] > 0.5 && u < v) {
                    edges_cuted.push_back(format_edge{u, v, cost});
                }
            }
        }

        bool is_valid_solution = false;
        double real_fit = final_status.best_fitness;

        if (final_status.best_fitness <= 1) {
            is_valid_solution = true;
            real_fit = final_status.best_fitness * instance.cumulative_edge_cost;
        }

        write_in_file (
            output_file_name, 
            instance.num_nodes, 
            instance.num_edges, 
            instance.num_terminals,
            is_valid_solution,
            real_fit,
            final_status.last_update_iteration,
            final_status.last_update_time,
            final_status.current_iteration,
            final_status.current_time,
            num_edges_cut,
            edges_cuted
        );
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}


void execute_coloracao(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing Decoder <coloracao>" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file);

        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] = BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {max_run_time};

        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;

        MCP_Decoder_Multiple_Thresholds decoder(instance);

        unsigned chromossome_size = instance.num_nodes;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Multiple_Thresholds> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

        algorithm.reset(); // chama #initialize(true)

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time << "s..." << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness
        << endl;

        ////////////////////////////////////////
        // Save results in file
        ////////////////////////////////////////

        // auxiliar 
        unsigned edge_index, v;

        /// Set of cuts
        std::vector<std::vector<MCP_Decoder_Multiple_Thresholds::edge>> set_cuts;

        /// Number of cuts an edge is part of
        std::vector<unsigned> num_cuts_edge(instance.num_edges, 0);

        /// Queue of nodes found
        std::queue<unsigned> queue;

        /// Marker of nodes found
        std::vector<bool> visited(instance.num_nodes, false);
        
        /////////////////////////////////////////////////
        // Breadth-first search to find the nodes
        // of the same group reachable from the terminal
        /////////////////////////////////////////////////

        /// Stores the group that each node belongs to
        std::vector<int> group(instance.num_nodes, -1);
        
        /// Compute group for non-terminal nodes
        for (unsigned i = 0; i < instance.num_nodes; i++) {
            group[i] = std::floor(final_status.best_chromosome[i] * instance.num_terminals);
        }

        /// Compute group for terminal nodes
        for (unsigned i = 0; i < instance.num_terminals; i++) {
            unsigned s = instance.terminals[i];
            group[s] = i;
        }

        for (unsigned s: instance.terminals) {
                
            visited[s] = true;
            queue.push(s);

            /// Store the nodes rechable from s using
            /// nodes that are part of the same group
            std::vector<MCP_Decoder_Multiple_Thresholds::edge> set_edges_out_group;

            while (!queue.empty()) {

                unsigned u = queue.front();
                queue.pop();
                
                /// Get the list of edges leaving u
                const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

                for (unsigned i = 0; i < u_list.size(); i++){
                    v = u_list[i].dst;

                    if (group[v] == group[u]) {
                        if (visited[v] == false) {
                            visited[v] = true;
                            queue.push(v);
                        }
                    }
                    else {
                        /// Adds the arc to the set
                        set_edges_out_group.push_back({u, v, u_list[i].cost, i});

                        /// Compute the edge index
                        edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[u] + i];
                        
                        /// Increases the number of cuts this edge participates in
                        num_cuts_edge[edge_index]++;
                    }
                }
            }

            set_cuts.push_back(set_edges_out_group);
        }

        /////////////////////////////////////////////
        // calculates the actual cost of all cuts
        // minus the cost of the highest value cut
        /////////////////////////////////////////////

        /// Marking value for edges already considered in a cut
        unsigned IGNORE = -1;
        unsigned num_edges_cut = 0;

        for (unsigned i = 0; i < set_cuts.size(); i++) {
            
            const std::vector<MCP_Decoder_Multiple_Thresholds::edge>& cut_edges = set_cuts[i];

            for (MCP_Decoder_Multiple_Thresholds::edge e: cut_edges) {
                edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[e.src] + e.edge_index];

                if (num_cuts_edge[edge_index] == 1)
                    num_edges_cut++;

                else if (num_cuts_edge[edge_index] == 2) {
                    num_cuts_edge[edge_index] = IGNORE;
                    num_edges_cut++; // non-contabilize reverse edge
                }
            }
        }

        vector<format_edge> edges_cuted;

        for (unsigned i = 0; i < set_cuts.size(); i++) {
            
            const std::vector<MCP_Decoder_Multiple_Thresholds::edge>& cut_edges = set_cuts[i];

            for (MCP_Decoder_Multiple_Thresholds::edge e: cut_edges) {
                edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[e.src] + e.edge_index];

                if (num_cuts_edge[edge_index] == 1)
                    edges_cuted.push_back(format_edge{e.src, e.dst, e.cost});

                else if (num_cuts_edge[edge_index] == IGNORE) {
                    num_cuts_edge[edge_index] = 2; // non-contabilize reverse edge
                    edges_cuted.push_back({e.src, e.dst, e.cost});
                }
            }
        }


        write_in_file (
            output_file_name, 
            instance.num_nodes, 
            instance.num_edges, 
            instance.num_terminals,
            true, // this decoder always obtains a valid solution
            final_status.best_fitness,
            final_status.last_update_iteration,
            final_status.last_update_time,
            final_status.current_iteration,
            final_status.current_time,
            num_edges_cut,
            edges_cuted
        );
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}



//-------------------------------[ Decoder execution ]------------------------------------//

void execute_kruskal(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing Decoder <kruskal" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file);


        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] =
            BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {max_run_time};

        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;

        MCP_Decoder_Kruskal decoder(instance);

        unsigned chromossome_size = instance.num_edges;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Kruskal> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

        algorithm.reset(); // chama #initialize(true)

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time << "..." << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness
        << endl;

        ofstream output_file(output_file_name);

        if (!output_file.is_open()) {
            cerr << "Erro ao abrir o arquivo " << output_file_name << endl;
        }

        ////////////////////////////////////////
        // Processing best solution
        // and save results in file
        ////////////////////////////////////////

        unsigned num_edges_cut = 0;

        /// Copy vector of edges
        vector<MCP_Decoder_Kruskal::edge> edges = decoder.list_edges;

        std::vector<unsigned> component(instance.num_nodes, 0);
        std::vector<int> terminal(instance.num_nodes, -1);
        std::vector<unsigned> rank(instance.num_nodes, 0);

        /// Mark who the component is and the terminal that each node is part of
        for (unsigned v = 0; v < instance.num_nodes; v++) {
            component[v] = v;
            terminal[v] = -1;
            rank[v] = 0;
        }

        for (unsigned s : instance.terminals) {
            terminal[s] = s;
        }

        
        BRKGA::Chromosome aux_chromosome(instance.num_edges);

        for (unsigned i = 0; i < instance.num_edges; i++) {
            aux_chromosome[i] = final_status.best_chromosome[i];
        }
        
        /// Order the chromosomesome based on the value of the alleles.
        /// Modifies the position of the edges together
        decoder.quick_sort(aux_chromosome, edges, 0, final_status.best_chromosome.size()-1);

        /// Auxiliary variables
        unsigned u, v, cost;
        unsigned cu, cv;

        vector<format_edge> edges_cuted;

        /// Traverses the edge vector by adding the edges that
        /// do not join components with different terminals.
        /// In the opposite case, the edge is added to the cut.
        for (unsigned i = 0; i < instance.num_edges; i++)
        {
            u = edges[i].src;
            v = edges[i].dst;
            cost = edges[i].cost;

            cu = decoder.find_element(component, u);
            cv = decoder.find_element(component, v);

            if (cu != cv)
            {
                if (terminal[cu] == -1 || terminal[cv] == -1)
                    decoder.union_elements(component, terminal, rank, u, v);
                else {
                    num_edges_cut++;
                    edges_cuted.push_back({u, v, cost});
                }
                    
            }
        }

        write_in_file (
            output_file_name, 
            instance.num_nodes, 
            instance.num_edges, 
            instance.num_terminals,
            true, // this decoder always obtains a valid solution
            final_status.best_fitness,
            final_status.last_update_iteration,
            final_status.last_update_time,
            final_status.current_iteration,
            final_status.current_time,
            num_edges_cut,
            edges_cuted
        );
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}

//-------------------------------[ Decoder execution ]------------------------------------//

void execute_kruskal_pert(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing Decoder <kruskalpert>" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file);


        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] =
            BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {max_run_time};

        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;

        MCP_Decoder_Kruskal_Pertubation decoder(instance);

        unsigned chromossome_size = instance.num_edges;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Kruskal_Pertubation> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

        // algorithm.reset(); // chama #initialize(true)

        // BRKGA::Chromosome initial_chromosome(chromossome_size, 0.5);
        
        // // {
        // algorithm.setInitialPopulation(
        //     vector<BRKGA::Chromosome>(1, initial_chromosome));
        // // }

        string instance_name = instance_file.substr(instance_file.length() - 9, 6);

        string caminho_arquivo_solucao_isolation = "../instances/concentric_sol/isolation_mcortes2/" + instance_name + ".sol";

        // {
        algorithm.setInitialPopulation(
            vector<BRKGA::Chromosome>(1, gerador_cromossomo_krp(caminho_arquivo_solucao_isolation)));
        // }

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time << "..." << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness
        << endl;

        ////////////////////////////////////////
        // Processing best solution
        // and save results in file
        ////////////////////////////////////////

        vector<format_edge> edges_cuted;

        /// Copy vector of edges
        vector<MCP_Decoder_Kruskal_Pertubation::edge> edges = decoder.list_edges;
        vector<double> perturbed_costs(instance.num_edges);

        std::vector<unsigned> component(instance.num_nodes, 0);
        std::vector<int> terminal(instance.num_nodes, -1);
        std::vector<unsigned> rank(instance.num_nodes, 0);

        /// Mark who the component is and the terminal that each node is part of
        for (unsigned v = 0; v < instance.num_nodes; v++) {
            component[v] = v;
            terminal[v] = -1;
            rank[v] = 0;
        }

        for (unsigned s : instance.terminals) {
            terminal[s] = s;
        }

        /// Upsets the cost of edges
        for (unsigned i = 0; i < instance.num_edges; i++) {
            perturbed_costs[i] = edges[i].cost * final_status.best_chromosome[i];
        }

        unsigned num_edges_cut = 0;

        /// Order the chromosomesome based on the value of the alleles.
        /// Modifies the position of the edges together
        decoder.quick_sort(perturbed_costs, edges, 0, perturbed_costs.size()-1);

        /// Auxiliary variables
        unsigned u, v, cost;
        unsigned cu, cv;

        /// Traverses the edge vector by adding the edges that
        /// do not join components with different terminals.
        /// In the opposite case, the edge is added to the cut.
        for (unsigned i = 0; i < instance.num_edges; i++)
        {
            u = edges[i].src;
            v = edges[i].dst;
            cost = edges[i].cost;

            cu = decoder.find_element(component, u);
            cv = decoder.find_element(component, v);

            if (cu != cv)
            {
                if (terminal[cu] == -1 || terminal[cv] == -1)
                    decoder.union_elements(component, terminal, rank, u, v);
                else {
                    // uses the original cost
                    num_edges_cut++;
                    edges_cuted.push_back({u, v, cost});
                }
            }
        }


        write_in_file (
            output_file_name, 
            instance.num_nodes, 
            instance.num_edges, 
            instance.num_terminals,
            true, // this decoder always obtains a valid solution
            final_status.best_fitness,
            final_status.last_update_iteration,
            final_status.last_update_time,
            final_status.current_iteration,
            final_status.current_time,
            num_edges_cut,
            edges_cuted
        );

    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}


//-------------------------------[ Decoder execution ]------------------------------------//

void execute_multiplos_cortes(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing Decoder <mcortes>" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file);


        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] =
            BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {max_run_time};


        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;

        MCP_Decoder_Cuts decoder(instance);

        unsigned chromossome_size = instance.num_edges;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Cuts> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

        // algorithm.reset(); // chama #initialize(true)

        BRKGA::Chromosome initial_chromosome(chromossome_size, 0.5);
        
        // {
        algorithm.setInitialPopulation(
            vector<BRKGA::Chromosome>(1, initial_chromosome));
        // }

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time << "..." << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        // BRKGA::Chromosome chromosome = {0.59878, 0.59878, 0.59878, 0.59878, 0.59878, 0.59878};
        // decoder.decode(chromosome, true);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness
        << endl;

        // BRKGA::Chromosome chromosome = final_status.best_chromosome;

        // double accumulated_cost_cuts = 0;

        // //////////////////////////////////////////////////
        // // creates a residual graph instance with the
        // // costs of the arcs perturbed by the chromosome
        // //////////////////////////////////////////////////

        // unsigned u, v, cost; 
        // unsigned edge_index;
        // MCP_Instance::edge edge_e;

        // // the n+1 vertex will be used as an auxiliary node to perform the cuts
        // Graph G(instance.num_nodes);

        // for (u = 0; u < instance.num_nodes; u++)
        // {
        //     for (unsigned i = 0; i < instance.G[u].size(); i++)
        //     {
        //         edge_e = instance.G[u][i];
        //         v = edge_e.dst;
        //         cost = edge_e.cost;

        //         if (u < v)
        //         {
        //             G.add_edge_with_reverse(u, v, (cost * chromosome[edge_index]), i);
        //             edge_index++;
        //         }
        //     }
        // }

        // //////////////////////////////////////////////////
        // // calculates the cuts that isolate
        // // each si terminal from the others
        // //////////////////////////////////////////////////

        // unsigned t = instance.num_nodes;

        // vector<vector<highest_push_relabel_max_flow::edge>> cuts;

        // for (unsigned si : instance.terminals)
        // {
        //     // Create hpr object
        //     highest_push_relabel_max_flow solver(G);

        //     for (unsigned sj : instance.terminals)
        //     {
        //         if (sj != si)
        //             solver.add_edge(sj, t, std::numeric_limits<double>::max(), 0);
        //     }

        //     // Run algorithm that calculates the maximum flow
        //     solver.solve(si, t);

        //     // Store de edges cut
        //     cuts.push_back(solver.get_edges_cut());
        // }

        // //////////////////////////////////////////////////
        // // Calculates the number of times 
        // // each arc appeared in a cut
        // //////////////////////////////////////////////////

        // /// Number of cuts an edge is part of
        // std::vector<int> num_cuts_edge(instance.num_edges, 0);

        // for (unsigned i = 0; i < cuts.size(); i++)
        // {
        //     for (highest_push_relabel_max_flow::edge e : cuts[i])
        //     {
        //         /// Compute the edge index
        //         edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[e.src] + e.index];

        //         /// Increases the number of cuts this edge participates in
        //         num_cuts_edge[edge_index]++;
        //     }
        // }

        // //////////////////////////////////////////////////
        // // Calculates the cost of all cuts.
        // // It also calculates the highest real cost cut (only
        // // considering arcs that are part of a single cut)
        // //////////////////////////////////////////////////

        // unsigned highest_cost = 0;
        // unsigned index_cut = 0;

        // for (unsigned i = 0; i < cuts.size(); i++)
        // {
        //     double cost_cut = 0;

        //     for (highest_push_relabel_max_flow::edge e : cuts[i])
        //     {
        //         /// Compute the edge index
        //         edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[e.src] + e.index];

        //         if (num_cuts_edge[edge_index] == 1)
        //             cost_cut += instance.G[e.src][e.index].cost;
        //     }

        //     if (cost_cut > highest_cost)
        //     {
        //         highest_cost = cost_cut;
        //         index_cut = i;
        //     }

        //     for (highest_push_relabel_max_flow::edge e : cuts[i])
        //     {
        //         /// Compute the edge index
        //         edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[e.src] + e.index];

        //         if (num_cuts_edge[edge_index] >= 2) {
        //             accumulated_cost_cuts += instance.G[e.src][e.index].cost;
        //             num_cuts_edge[edge_index] = -1;
        //         }
        //     }

        //     accumulated_cost_cuts += cost_cut;
        // }

        // typedef struct {int src; int dst; unsigned cost;} aux_edge;
        // vector<format_edge> edges_cuted;
        // unsigned num_edges_cut = 0;

        // // calcula numero de arcos no corte
        // for (unsigned i = 0; i < cuts.size(); i++)
        // {
        //     if (i != index_cut) {
        //         for (highest_push_relabel_max_flow::edge e : cuts[i])
        //         {
        //             /// Compute the edge index
        //             edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[e.src] + e.index];

        //             if (num_cuts_edge[edge_index] == 1) { // insere o arcos que fazem parte de um unico corte
        //                 edges_cuted.push_back(format_edge{static_cast<unsigned int>(e.src), static_cast<unsigned int>(e.dst), instance.G[e.src][e.index].cost});
        //                 num_edges_cut++;
        //             }
        //             if (num_cuts_edge[edge_index] == -1) { // insere o arcos que fazem parte de 2 ou mais cortes apenas uma vez
        //                 edges_cuted.push_back(format_edge{static_cast<unsigned int>(e.src), static_cast<unsigned int>(e.dst), instance.G[e.src][e.index].cost});
        //                 num_cuts_edge[edge_index] == -2; 
        //                 num_edges_cut++;
        //             }
        //         }
        //     }
        // }

        vector<format_edge> edges_cuted;
        unsigned num_edges_cut = 0;

        write_in_file (
            output_file_name, 
            instance.num_nodes, 
            instance.num_edges, 
            instance.num_terminals,
            true, // this decoder always obtains a valid solution
            final_status.best_fitness,
            final_status.last_update_iteration,
            final_status.last_update_time,
            final_status.current_iteration,
            final_status.current_time,
            num_edges_cut,
            edges_cuted
        );

    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}



void execute_coloracao2(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing Decoder <coloracao2>" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file);

        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] = BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {max_run_time};

        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;
        
        MCP_Decoder_Coloracao2 decoder(instance);

        unsigned chromossome_size = instance.num_nodes;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Coloracao2> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

        algorithm.reset(); // chama #initialize(true)

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time << "s..." << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness
        << endl;

        ////////////////////////////////////////
        // Save results in file
        ////////////////////////////////////////

        // auxiliar 
        unsigned edge_index, v;

        /// Set of cuts
        std::vector<std::vector<MCP_Decoder_Multiple_Thresholds::edge>> set_cuts;

        /// Number of cuts an edge is part of
        std::vector<unsigned> num_cuts_edge(instance.num_edges, 0);

        /// Queue of nodes found
        std::queue<unsigned> queue;

        /// Marker of nodes found
        std::vector<bool> visited(instance.num_nodes, false);
        
        /////////////////////////////////////////////////
        // Breadth-first search to find the nodes
        // of the same group reachable from the terminal
        /////////////////////////////////////////////////

        /// Stores the group that each node belongs to
        std::vector<int> group(instance.num_nodes, -1);
        
        /// Compute group for non-terminal nodes
        for (unsigned i = 0; i < instance.num_nodes; i++) {
            group[i] = std::floor(final_status.best_chromosome[i] * instance.num_terminals);
        }

        /// Compute group for terminal nodes
        for (unsigned i = 0; i < instance.num_terminals; i++) {
            unsigned s = instance.terminals[i];
            group[s] = i;
        }

        for (unsigned s: instance.terminals) {
                
            visited[s] = true;
            queue.push(s);

            /// Store the nodes rechable from s using
            /// nodes that are part of the same group
            std::vector<MCP_Decoder_Multiple_Thresholds::edge> set_edges_out_group;

            while (!queue.empty()) {

                unsigned u = queue.front();
                queue.pop();
                
                /// Get the list of edges leaving u
                const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

                for (unsigned i = 0; i < u_list.size(); i++){
                    v = u_list[i].dst;

                    if (group[v] == group[u]) {
                        if (visited[v] == false) {
                            visited[v] = true;
                            queue.push(v);
                        }
                    }
                    else {
                        /// Adds the arc to the set
                        set_edges_out_group.push_back({u, v, u_list[i].cost, i});

                        /// Compute the edge index
                        edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[u] + i];
                        
                        /// Increases the number of cuts this edge participates in
                        num_cuts_edge[edge_index]++;
                    }
                }
            }

            set_cuts.push_back(set_edges_out_group);
        }

        /////////////////////////////////////////////
        // calculates the actual cost of all cuts
        // minus the cost of the highest value cut
        /////////////////////////////////////////////

        /// Marking value for edges already considered in a cut
        unsigned IGNORE = -1;
        unsigned num_edges_cut = 0;

        for (unsigned i = 0; i < set_cuts.size(); i++) {
            
            const std::vector<MCP_Decoder_Multiple_Thresholds::edge>& cut_edges = set_cuts[i];

            for (MCP_Decoder_Multiple_Thresholds::edge e: cut_edges) {
                edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[e.src] + e.edge_index];

                if (num_cuts_edge[edge_index] == 1)
                    num_edges_cut++;

                else if (num_cuts_edge[edge_index] == 2) {
                    num_cuts_edge[edge_index] = IGNORE;
                    num_edges_cut++; // non-contabilize reverse edge
                }
            }
        }

        vector<format_edge> edges_cuted;

        for (unsigned i = 0; i < set_cuts.size(); i++) {
            
            const std::vector<MCP_Decoder_Multiple_Thresholds::edge>& cut_edges = set_cuts[i];

            for (MCP_Decoder_Multiple_Thresholds::edge e: cut_edges) {
                edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[e.src] + e.edge_index];

                if (num_cuts_edge[edge_index] == 1)
                    edges_cuted.push_back(format_edge{e.src, e.dst, e.cost});

                else if (num_cuts_edge[edge_index] == IGNORE) {
                    num_cuts_edge[edge_index] = 2; // non-contabilize reverse edge
                    edges_cuted.push_back({e.src, e.dst, e.cost});
                }
            }
        }


        write_in_file (
            output_file_name, 
            instance.num_nodes, 
            instance.num_edges, 
            instance.num_terminals,
            true, // this decoder always obtains a valid solution
            final_status.best_fitness,
            final_status.last_update_iteration,
            final_status.last_update_time,
            final_status.current_iteration,
            final_status.current_time,
            num_edges_cut,
            edges_cuted
        );
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}





void execute_threshold2(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing Decoder <threshold2>" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file);

        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] = BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {max_run_time};

        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;

        MCP_Decoder_Threshold2 decoder(instance);

        // BRKGA::Chromosome x = {0.897114, 0.489479, 0.111952, 0.475832, 0.824423, 0.73154, 0.057623, 0.481461, 0.72154, 0.581603, 0.672329, 0.661998, 0.91152, 0.518634, 0};

        // decoder.decode(x, true);

        unsigned chromossome_size = instance.num_edges;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Threshold2> algorithm (
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

        algorithm.reset(); // chama #initialize(true)

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time << "s..." << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness * instance.cumulative_edge_cost
        << endl;

        ////////////////////////////////////////
        // Save results in file
        ////////////////////////////////////////

        bool is_valid_solution = false;
        double real_fit = final_status.best_fitness;

        if (final_status.best_fitness <= 1) {
            is_valid_solution = true;
            real_fit = final_status.best_fitness * instance.cumulative_edge_cost;
        }

        vector<format_edge> edges_cuted;
        unsigned num_edges_cut = 0;

        write_in_file (
            output_file_name, 
            instance.num_nodes, 
            instance.num_edges, 
            instance.num_terminals,
            is_valid_solution,
            real_fit,
            final_status.last_update_iteration,
            final_status.last_update_time,
            final_status.current_iteration,
            final_status.current_time,
            num_edges_cut,
            edges_cuted
        );
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}


void execute_coloracao3(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing Decoder <coloracao3>" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file);

        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] = BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {max_run_time};

        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;
        
        MCP_Decoder_Coloracao3 decoder(instance);

        unsigned chromossome_size = instance.num_nodes;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Coloracao3> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

        // algorithm.reset(); // chama #initialize(true)

        string instance_name = instance_file.substr(instance_file.length() - 9, 6);

        string caminho_arquivo_solucao_isolation = "../instances/concentric_sol/isolation_mcortes2/" + instance_name + ".sol";

        // {
        algorithm.setInitialPopulation(
            vector<BRKGA::Chromosome>(1, gerador_cromossomo_col3(caminho_arquivo_solucao_isolation)));
        // }

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time << "s..." << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness
        << endl;

        // std::vector<BRKGA::fitness_t> crm = {0.671674, 0.736685, 0.405302, 0.172798, 0.903286, 0.549317, 0.548541, 0.0870224, 0.0157432};
        // decoder.decode(crm, true);

    
        vector<format_edge> edges_cuted;
        unsigned num_edges_cut = 0;

        write_in_file (
            output_file_name, 
            instance.num_nodes, 
            instance.num_edges, 
            instance.num_terminals,
            true, // this decoder always obtains a valid solution
            final_status.best_fitness,
            final_status.last_update_iteration,
            final_status.last_update_time,
            final_status.current_iteration,
            final_status.current_time,
            num_edges_cut,
            edges_cuted
        );
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}



void execute_threshold3(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing Decoder <threshold3>" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file);

        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] = BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {max_run_time};

        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;

        MCP_Decoder_Threshold3 decoder(instance);

        // decoder.decode(x, true);

        unsigned chromossome_size = instance.num_edges;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Threshold3> algorithm (
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

        // algorithm.reset(); // chama #initialize(true)

        string instance_name = instance_file.substr(instance_file.length() - 9, 6);

        string caminho_arquivo_solucao_isolation = "../instances/concentric_sol/isolation_mcortes2/" + instance_name + ".sol";

        // {
        algorithm.setInitialPopulation(
            vector<BRKGA::Chromosome>(1, gerador_cromossomo_th3(caminho_arquivo_solucao_isolation)));
        // }

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time << "s..." << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness
        << endl;

        ////////////////////////////////////////
        // Save results in file
        ////////////////////////////////////////

        bool is_valid_solution = true;

        vector<format_edge> edges_cuted;
        unsigned num_edges_cut = 0;

        write_in_file (
            output_file_name, 
            instance.num_nodes, 
            instance.num_edges, 
            instance.num_terminals,
            is_valid_solution,
            final_status.best_fitness,
            final_status.last_update_iteration,
            final_status.last_update_time,
            final_status.current_iteration,
            final_status.current_time,
            num_edges_cut,
            edges_cuted
        );
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}



void execute_multiplos_cortes2(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing Decoder <mcortes2>" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file);


        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] =
            BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {max_run_time};


        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;

        MCP_Decoder_Cuts2 decoder(instance);

        unsigned chromossome_size = instance.num_edges;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Cuts2> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

        // algorithm.reset(); // chama #initialize(true)

        // BRKGA::Chromosome initial_chromosome(chromossome_size, 0.5);
        
        // // {
        // algorithm.setInitialPopulation(
        //     vector<BRKGA::Chromosome>(1, initial_chromosome));
        // // }

        string instance_name = instance_file.substr(instance_file.length() - 9, 6);

        string caminho_arquivo_solucao_isolation = "../instances/concentric_sol/isolation_mcortes2/" + instance_name + ".sol";

        // {
        algorithm.setInitialPopulation(
            vector<BRKGA::Chromosome>(1, gerador_cromossomo_krp(caminho_arquivo_solucao_isolation)));
        // }

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time << "..." << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        // BRKGA::Chromosome chromosome = {0.59878, 0.59878, 0.59878, 0.59878, 0.59878, 0.59878};
        // decoder.decode(chromosome, true);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness
        << endl;

        vector<format_edge> edges_cuted;
        unsigned num_edges_cut = 0;

        write_in_file (
            output_file_name, 
            instance.num_nodes, 
            instance.num_edges, 
            instance.num_terminals,
            true, // this decoder always obtains a valid solution
            final_status.best_fitness,
            final_status.last_update_iteration,
            final_status.last_update_time,
            final_status.current_iteration,
            final_status.current_time,
            num_edges_cut,
            edges_cuted
        );

    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}