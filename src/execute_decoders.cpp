#include "mcp/mcp_instance.hpp"
#include "brkga_mp_ipr.hpp"
#include "brkga_mp_ipr/chromosome.hpp"

#include "write_file.hpp"
#include "execute_decoders.hpp"

#include "decoders/mcp_decoder_single_t.hpp"
#include "decoders/mcp_decoder_multi_t.hpp"
#include "decoders/mcp_decoder_kruskal.hpp"
#include "decoders/mcp_decoder_kruskal_pert.hpp"
#include "decoders/mcp_decoder_cuts.hpp"
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

void execute_single_t(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing MCP_Decoder_Single_Threshold" << endl;

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

        for (unsigned u = 1; u <= instance.num_nodes; u++) {
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


        if (final_status.best_fitness <= 1) {
            is_valid_solution = true;
        }

        write_in_file (
            output_file_name, 
            instance.num_nodes, 
            instance.num_edges, 
            instance.num_terminals,
            is_valid_solution,
            final_status.best_fitness * instance.cumulative_edge_cost,
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


void execute_multiple_t(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing MCP_Decoder_Multiple_Thresholds" << endl;

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

        unsigned chromossome_size = instance.num_nodes + 1;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Multiple_Thresholds> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );

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
        cout << "Executing MCP_Decoder_Kruskal" << endl;

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
        cout << "Executing MCP_Decoder_Kruskal" << endl;

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

void execute_cuts(
    const unsigned seed,
    const string config_file,
    const unsigned max_run_time,
    const string instance_file,
    const string output_file_name,
    const unsigned num_threads
) {
    try {
        cout << "Executing MCP_Decoder_Cuts" << endl;

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

        BRKGA::Chromosome chromosome = final_status.best_chromosome;

        double accumulated_cost_cuts = 0;

        //////////////////////////////////////////////////
        // creates a residual graph instance with the
        // costs of the arcs perturbed by the chromosome
        //////////////////////////////////////////////////

        unsigned u, v, cost, edge_index = 0;
        MCP_Instance::edge edge_e;

        // the n+1 vertex will be used as an auxiliary node to perform the cuts
        Graph G(instance.num_nodes + 1);

        for (u = 1; u <= instance.num_nodes; u++)
        {
            for (unsigned i = 0; i < instance.G[u].size(); i++)
            {
                edge_e = instance.G[u][i];
                v = edge_e.dst;
                cost = edge_e.cost;

                if (u < v)
                {
                    G.add_edge_with_reverse(u, v, (cost * chromosome[edge_index]), i);
                    edge_index++;
                }
            }
        }

        //////////////////////////////////////////////////
        // calculates the cuts that isolate
        // each si terminal from the others
        //////////////////////////////////////////////////

        unsigned t = instance.num_nodes + 1;

        vector<vector<highest_push_relabel_max_flow::edge>> cuts;

        for (unsigned si : instance.terminals)
        {
            // Create hpr object
            highest_push_relabel_max_flow solver(G);

            for (unsigned sj : instance.terminals)
            {
                if (sj != si)
                {
                    solver.add_edge(sj, t, std::numeric_limits<double>::max(), 0);
                }
            }

            // Run algorithm that calculates the maximum flow
            solver.solve(si, t);

            // Store de edges cut
            cuts.push_back(solver.get_edges_cut());
        }

        //////////////////////////////////////////////////
        // Calculates the number of times 
        // each arc appeared in a cut
        //////////////////////////////////////////////////

        /// Number of cuts an edge is part of
        std::vector<unsigned> num_cuts_edge(instance.num_edges, 0);

        for (unsigned i = 0; i < cuts.size(); i++)
        {
            for (highest_push_relabel_max_flow::edge e : cuts[i])
            {
                /// Compute the edge index
                edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[e.src] + e.index];

                /// Increases the number of cuts this edge participates in
                num_cuts_edge[edge_index]++;
            }
        }

        //////////////////////////////////////////////////
        // Calculates the cost of all cuts.
        // It also calculates the highest real cost cut (only
        // considering arcs that are part of a single cut)
        //////////////////////////////////////////////////

        unsigned highest_cost = 0;
        unsigned index_cut = 0;

        for (unsigned i = 0; i < cuts.size(); i++)
        {
            double cost_cut = 0;

            for (highest_push_relabel_max_flow::edge e : cuts[i])
            {
                /// Compute the edge index
                edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[e.src] + e.index];

                if (num_cuts_edge[edge_index] == 1)
                    cost_cut += instance.G[e.src][e.index].cost;
                if (num_cuts_edge[edge_index] == 2) {
                    accumulated_cost_cuts += instance.G[e.src][e.index].cost;
                    num_cuts_edge[edge_index] = -1;
                }
            }

            accumulated_cost_cuts += cost_cut;

            if (cost_cut > highest_cost)
            {
                highest_cost = cost_cut;
                index_cut = i;
            }
        }

        typedef struct {int src; int dst; unsigned cost;} aux_edge;
        vector<format_edge> edges_cuted;
        unsigned num_edges_cut = 0;

        // calcula numero de arcos no corte
        for (unsigned i = 0; i < cuts.size(); i++)
        {
            if (i != index_cut) {
                for (highest_push_relabel_max_flow::edge e : cuts[i])
                {
                    /// Compute the edge index
                    edge_index = decoder.position_edge_vector[decoder.init_adjacency_list[e.src] + e.index];

                    if (num_cuts_edge[edge_index] == 1) {
                        edges_cuted.push_back(format_edge{static_cast<unsigned int>(e.src), static_cast<unsigned int>(e.dst), instance.G[e.src][e.index].cost});
                        num_edges_cut++;
                    }
                    if (num_cuts_edge[edge_index] == -1) {
                        edges_cuted.push_back(format_edge{static_cast<unsigned int>(e.src), static_cast<unsigned int>(e.dst), instance.G[e.src][e.index].cost});
                        num_edges_cut++;
                    }
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