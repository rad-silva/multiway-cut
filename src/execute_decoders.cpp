#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr.hpp"
#include "brkga_mp_ipr/chromosome.hpp"

#include "max_flow/graph.hpp"
#include "decoders/mcp_decoder_threshold.hpp"
#include "decoders/mcp_decoder_multiple_thresholds.hpp"
#include "decoders/mcp_decoder_kruskal.hpp"
#include "decoders/mcp_decoder_kruskal_perturbation.hpp"
#include "decoders/mcp_decoder_cuts.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <queue>

using namespace std;

//-------------------------------[ Main ]------------------------------------//

void execute_single_t(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const unsigned type_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing MCP_Decoder_Single_Threshold" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file, type_file);

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

        MCP_Decoder_Single_Threshold decoder(instance);

        unsigned chromossome_size = instance.num_edges;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Single_Threshold> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

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
        // Save results in file
        ////////////////////////////////////////

        unsigned edge_index, v, cost;

        ofstream output_file(output_file_name);

        if (!output_file.is_open()) {
            cerr << "Erro ao abrir o arquivo " << output_file_name << endl;
        }

        if (final_status.best_fitness <= 1) {
            output_file << "Valid Solution: yes" << "\n";
            output_file << "Best cost: " << final_status.best_fitness * instance.cumulative_edge_cost << "\n";
        }
        else {
            output_file << "Valid Solution: no" << "\n";
            output_file << "Best cost: " << final_status.best_fitness << "\n";
        }

        unsigned num_edges_cut = 0;

        for (unsigned i = 0; i < final_status.best_chromosome.size(); i++) {
            if (final_status.best_chromosome[i] > 0.5) {
                num_edges_cut++;
            }
        }

        output_file << "Last update iteration: " << final_status.last_update_iteration << "\n";
        output_file << "Last update time: " << final_status.last_update_time << "\n";
        output_file << "Final iteration: " << final_status.current_iteration << "\n";
        output_file << "Final time: " << final_status.current_time << "\n";

        output_file << "Num edges cut: " << num_edges_cut << "\n";

        output_file << "Edges removed: " << "\n";

        for (unsigned u = 1; u <= instance.num_nodes; u++) {
            std::vector<MCP_Instance::edge> u_list = instance.G[u];

            for (unsigned i = 0; i < u_list.size(); i++) {
                edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[u] + i];
                v = u_list[i].dst;
                cost = u_list[i].cost;

                if (final_status.best_chromosome[edge_index] > 0.5 && u < v) {
                    output_file << u << " " << v << " " << cost << "\n";
                }
            }
        }
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}



//-------------------------------[ Main ]------------------------------------//



void execute_multiple_t(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const unsigned type_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing MCP_Decoder_Multiple_Thresholds" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file, type_file);

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

        MCP_Decoder_Multiple_Thresholds decoder(instance);

        unsigned chromossome_size = instance.num_nodes + 1;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Multiple_Thresholds> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

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
        // Save results in file
        ////////////////////////////////////////

        unsigned edge_index, v;

        ofstream output_file(output_file_name);

        if (!output_file.is_open()) {
            cerr << "Erro ao abrir o arquivo " << output_file_name << endl;
        }

        output_file << "Valid Solution: yes" << "\n";
        output_file << "Best cost: " << final_status.best_fitness << "\n";

        output_file << "Last update iteration: " << final_status.last_update_iteration << "\n";
        output_file << "Last update time: " << final_status.last_update_time << "\n";
        output_file << "Final iteration: " << final_status.current_iteration << "\n";
        output_file << "Final time: " << final_status.current_time << "\n";

        /// Set of cuts
        std::vector<std::vector<MCP_Decoder_Multiple_Thresholds::edge>> set_cuts;

        /// Number of cuts an edge is part of
        std::vector<unsigned> num_cuts_edge(instance.num_edges, 0);

        /// Queue of nodes found
        std::queue<unsigned> queue;

        /// Marker of nodes found
        std::vector<bool> visited(instance.num_nodes + 1, false);
        
        /////////////////////////////////////////////////
        // Breadth-first search to find the nodes
        // of the same group reachable from the terminal
        /////////////////////////////////////////////////

        /// Stores the group that each node belongs to
        std::vector<int> group(instance.num_nodes + 1, -1);
        
        /// Compute group for non-terminal nodes
        for (unsigned i = 1; i <= instance.num_nodes; i++) {
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
                        set_edges_out_group.push_back({u, v, u_list[i].cost});

                        /// Compute the edge index
                        edge_index = decoder.position_edge_vector[decoder.init_adjascency_list[u] + i];
                        
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
                edge_index = decoder.position_edge_vector[decoder.init_adjascency_list[e.src] + decoder.get_edge_index(e.src, e.dst)];

                if (num_cuts_edge[edge_index] == 1)
                    num_edges_cut++;

                else if (num_cuts_edge[edge_index] == 2) {
                    num_cuts_edge[edge_index] = IGNORE;
                    num_edges_cut++; // non-contabilize reverse edge
                }
            }
        }

        output_file << "Num edges cut:" << " " << num_edges_cut << "\n";

        output_file << "Edges removed:\n";

        for (unsigned i = 0; i < set_cuts.size(); i++) {
            
            const std::vector<MCP_Decoder_Multiple_Thresholds::edge>& cut_edges = set_cuts[i];

            for (MCP_Decoder_Multiple_Thresholds::edge e: cut_edges) {
                edge_index = decoder.position_edge_vector[decoder.init_adjascency_list[e.src] + decoder.get_edge_index(e.src, e.dst)];

                if (num_cuts_edge[edge_index] == 1)
                    output_file << e.src << " " << e.dst << " " << e.cost << "\n";

                else if (num_cuts_edge[edge_index] == IGNORE) {
                    num_cuts_edge[edge_index] = 2; // non-contabilize reverse edge
                    output_file << e.src << " " << e.dst << " " << e.cost << "\n";
                }
            }
        }
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}



//-------------------------------[ Main ]------------------------------------//

void execute_kruskal(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const unsigned type_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing MCP_Decoder_Kruskal" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file, type_file);


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

        output_file << "Valid Solution: yes" << "\n";
        output_file << "Best cost: " << final_status.best_fitness << "\n";

        output_file << "Last update iteration: " << final_status.last_update_iteration << "\n";
        output_file << "Last update time: " << final_status.last_update_time << "\n";
        output_file << "Final iteration: " << final_status.current_iteration << "\n";
        output_file << "Final time: " << final_status.current_time << "\n";

        unsigned num_edges_cut = 0;

        /// Copy vector of edges
        vector<MCP_Decoder_Kruskal::edge> edges = decoder.list_edges;
        vector<MCP_Decoder_Kruskal::edge> edges_removed;

        std::vector<unsigned> component(instance.num_nodes + 1, 0);
        std::vector<int> terminal(instance.num_nodes + 1, -1);
        std::vector<unsigned> rank(instance.num_nodes + 1, 0);

        /// Mark who the component is and the terminal that each node is part of
        for (unsigned v = 1; v <= instance.num_nodes; v++) {
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
                    edges_removed.push_back({u, v, cost});
                }
                    
            }
        }

        output_file << "Num edges cut:" << " " << num_edges_cut << "\n";

        output_file << "Edges removed:\n";

        for (MCP_Decoder_Kruskal::edge e: edges_removed) {
            output_file << e.src << " " << e.dst << " " << e.cost << "\n";
        }

    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}


//-------------------------------[ Main ]------------------------------------//


void execute_kruskal_pert(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const unsigned type_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing MCP_Decoder_Kruskal" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file, type_file);


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

        output_file << "Valid Solution: yes" << "\n";
        output_file << "Best cost: " << final_status.best_fitness << "\n";

        output_file << "Last update iteration: " << final_status.last_update_iteration << "\n";
        output_file << "Last update time: " << final_status.last_update_time << "\n";
        output_file << "Final iteration: " << final_status.current_iteration << "\n";
        output_file << "Final time: " << final_status.current_time << "\n";

        vector<MCP_Decoder_Kruskal_Pertubation::edge> edges_removed;

        /// Copy vector of edges
        vector<MCP_Decoder_Kruskal_Pertubation::edge> edges = decoder.list_edges;
        vector<double> perturbed_costs(instance.num_edges);

        std::vector<unsigned> component(instance.num_nodes + 1, 0);
        std::vector<int> terminal(instance.num_nodes + 1, -1);
        std::vector<unsigned> rank(instance.num_nodes + 1, 0);

        /// Mark who the component is and the terminal that each node is part of
        for (unsigned v = 1; v <= instance.num_nodes; v++) {
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
                    edges_removed.push_back({u, v, cost});
                }
            }
        }

        output_file << "Num edges cut:" << " " << num_edges_cut << "\n";

        output_file << "Edges removed:\n";

        for (MCP_Decoder_Kruskal_Pertubation::edge e: edges_removed) {
            output_file << e.src << " " << e.dst << " " << e.cost << "\n";
        }

    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}


//-------------------------------[ Main ]------------------------------------//


void execute_cuts(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const unsigned type_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing MCP_Decoder_Kruskal" << endl;

        ////////////////////////////////////////
        // Read the instance
        ////////////////////////////////////////

        cout << "Reading data..." << endl;

        auto instance = MCP_Instance(instance_file, type_file);


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

    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
    }
}