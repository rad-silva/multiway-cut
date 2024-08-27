#include "geradores/geradores_cromossomo.hpp"
#include "brkga_mp_ipr/chromosome.hpp"
#include "mcp/mcp_instance.hpp"

#include <queue>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;


BRKGA::Chromosome gerador_cromossomo_th3(string filename) {

    cout << filename << endl;
    
    ifstream file(filename, ios::in);

    if (!file) {
        throw runtime_error("Cannot open instance file");
    }

    string line;
    string mark, discard;
    unsigned u, v, edge_index, num_nodes, num_edges;
    double cost;

    file >> mark >> discard >> num_nodes >> num_edges;

    // Ignorar a primeira linha
    getline(file, line);

    BRKGA::Chromosome chromosome(num_edges, 0.0);

    // Lê cada linha até o final do arquivo
    while (getline(file, line)) {

        if (line.empty()) {
            continue; 
        }

        istringstream iss(line);
        if (iss >> mark >> u >> v >> cost >> edge_index) {
            chromosome[edge_index] = 0.9;
        }
    }

    return chromosome;
}


BRKGA::Chromosome gerador_cromossomo_krp(string filename) {

    cout << filename << endl;
    
    ifstream file(filename, ios::in);

    if (!file) {
        throw runtime_error("Cannot open instance file");
    }

    string line;
    string mark, discard;
    unsigned u, v, edge_index, num_nodes, num_edges;
    double cost;

    file >> mark >> discard >> num_nodes >> num_edges;

    // Ignorar a primeira linha
    getline(file, line);

    BRKGA::Chromosome chromosome(num_edges, 0.9);

    // Lê cada linha até o final do arquivo
    while (getline(file, line)) {

        if (line.empty()) {
            continue; 
        }

        istringstream iss(line);
        if (iss >> mark >> u >> v >> cost >> edge_index) {
            chromosome[edge_index] = 0.0;
        }
    }

    return chromosome;
}


BRKGA::Chromosome gerador_cromossomo_col3(string filename) {

    cout << filename << endl;
    
    ifstream file(filename, ios::in);

    if (!file) {
        throw runtime_error("Cannot open instance file");
    }

    string line;
    string mark, discard;
    unsigned u, v, edge_index, num_nodes, num_edges;
    double cost;

    file >> mark >> discard >> num_nodes >> num_edges;

    // Ignorar a primeira linha
    getline(file, line);

    std::vector<bool> removed_edge(num_edges, false);

    /// Marca as arestas que foram removidas

    while (getline(file, line)) {

        if (line.empty()) {
            continue; 
        }

        istringstream iss(line);
        if (iss >> mark >> u >> v >> cost >> edge_index) {
            removed_edge[edge_index] = true;
        }
    }

    /// Constrói um grafo a aprtir da instancia base

    string instance_path = "../instances/concentric_inst/all/" + filename.substr(filename.length() - 10, 6) + ".gr";

    cout << instance_path << endl;

    auto instance = MCP_Instance(instance_path);

    //
    std::vector<unsigned> init_adjacency_list(instance.num_nodes, -1);

    // Posição de cada aresta no vetor de arestas (mapeaia 2*m -> m)
    std::vector<unsigned> position_edge_vector(instance.num_edges * 2, -1);

    unsigned list_position = 0;
    unsigned edge_position = 0;
    
    for (u = 0; u < instance.num_nodes; u++) {
        init_adjacency_list[u] = list_position;

        std::vector<MCP_Instance::edge> u_list = instance.G[u];

        for (unsigned i = 0; i < u_list.size(); i++) {
            v = u_list[i].dst;

            if (u < v) {
                position_edge_vector[list_position + i] = edge_position;
                edge_position++;
            }
            else { // A posição de (u,v) no vetor já foi determinada previamente a partir de (v,u)
                position_edge_vector[list_position + i] = 
                    position_edge_vector[init_adjacency_list[v] + instance.get_edge_index(v,u)];
            }
        }

        list_position += u_list.size();
    }

    // Armazena os nós visitados (cada nó recebe o índice do terminal (partição) que o alcançou)
    std::vector<unsigned> visited_node(instance.num_nodes, -1);

    // Fila que armazena os nós encontrados
    std::queue<unsigned> queue;
    

    /// Busca em largura para atribuir a partição que cada nó pertence

    for (unsigned terminal_index = 0; terminal_index < instance.num_terminals; terminal_index++) {
        
        unsigned s = instance.terminals[terminal_index];
        visited_node[s] = terminal_index;
        queue.push(s);

        while (!queue.empty()) {

            u = queue.front();

            queue.pop();
            
            /// Obtem a lista de arestas que saem de u
            const std::vector<MCP_Instance::edge>& u_list = instance.G[u];

            for (unsigned i = 0; i < u_list.size(); i++) {

                v = u_list[i].dst;

                edge_index = position_edge_vector[init_adjacency_list[u] + i];

                if (removed_edge[edge_index] == false && visited_node[v] == -1) { 
                    visited_node[v] = terminal_index;
                    queue.push(v);
                }
            }
        }
    }

    /// Cria um cromossomo mapeando o índice dos terminal de cada nó para uma fração no intervalo [0,1)

    BRKGA::Chromosome chromosome(instance.num_nodes, 0);

    for (u = 0; u < instance.num_nodes; u++) {
        chromosome[u] = ((double)visited_node[u] / (double)instance.num_terminals) + ((double)1 / (2*instance.num_terminals));
    }

    return chromosome;
}

