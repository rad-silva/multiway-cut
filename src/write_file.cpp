#include "write_file.hpp"

//---------------------------[ write solution in file]---------------------------------//

void write_in_file (
    string file_name,
    unsigned num_nodes,
    unsigned num_edges,
    unsigned num_terminals,
    bool is_valid_solution,
    double best_fitness,
    long last_update_iteration,
    std::chrono::duration<double> last_update_time,
    long current_iteration,
    std::chrono::duration<double> current_time,
    unsigned num_edges_cuted,
    vector<format_edge>& edges_cuted
) {
    ofstream file(file_name);

    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo " << file_name << "\n";
    }

    file << "Nodes: " << num_nodes << "\n";
    file << "Edges: " << num_edges << "\n";
    file << "Terminals: " << num_terminals << "\n";

    if (is_valid_solution)
        file << "Valid Solution: true" << "\n";
    else
        file << "Valid Solution: false" << "\n";

    file << "Best cost: " << best_fitness << "\n";
    file << "Num edges cut: " << num_edges_cuted << "\n";

    file << "Last update iteration: " << last_update_iteration << "\n";
    file << "Last update time: " << last_update_time.count() << "\n";

    file << "Final iteration: " << current_iteration << "\n";
    file << "Final time: " << current_time.count() << "\n";

    file << "Edges cuted:" << "\n";
    for (format_edge e: edges_cuted) {
        file << e.src << " " << e.dst << " " << e.cost << "\n";
    }
}