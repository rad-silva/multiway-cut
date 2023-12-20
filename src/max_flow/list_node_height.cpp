#include <vector>
#include <iostream>
#include "list_node_height.hpp"

list_node_height::list_node_height(int n)
: num_nodes(n)
{
    list_height = std::vector<int>(2*(num_nodes+1), end_link);
    node = std::vector<node_link>(num_nodes+1);
}


void list_node_height::initialize_all_height(int h)
{
    // encadeia todos os nós na lista h
    // 0-1-2-3-...-n-n+1
    for(int i = 1; i <= num_nodes; i++) {
        node[i].height = h;
        node[i].prev = i-1;
        node[i].next = i+1;
    }

    // atualiza as ligações dos nós que estão nas pontas da lista
    // (end_link)-1-2-3-...-n-(end_link)
    node[1].prev = end_link;
    node[num_nodes].next = end_link;

    // atualiza a quem é a cabeça da lista de altura h
    list_height[h] = 1;
};


void list_node_height::remove_h(int i) {
    // verifica se o nó é válido
    if (i < 1 || i > num_nodes) {
        return;
    }

    // verifica se o nó não pertence a nenhuma altura
    if (node[i].height == -1)
        return;

    if (node[i].prev == end_link) {                
        // i é o primeiro nó da lista
        // Atualiza o nó cabeça em list_height
        list_height[node[i].height] = node[i].next;

        // Possíveis casos:
        // list[h]->i-j-k  ==>  list[h]->j-k
        // list[h]->i-(end_link) ==> list[h]->(end_link)

    } else {    
        // i não é o primeiro nó da lista
        // Faz com que o antecessor de i se ligue ao sucessor de i
        node[node[i].prev].next = node[i].next;

        // Possíveis casos:
        // list[h]->r-i-j  ==>  list[h]->r-j
        // list[h]->r-i-(end_link) ==> list[h]->r-(end_link)
    }

    if (node[i].next == end_link) {  
        // i é o último nó da lista
        // Não é necessário atualizar o próximo nó (já tratado em algum if anterior)
    } else {
        // i não é o último nó da lista
        // Liga o próximo nó ao antecessor de i       
        node[node[i].next].prev = node[i].prev;
    }
    
    node[i].prev = end_link;
    node[i].next = end_link;
    node[i].height = -1;
}

// ver forma de utilizar essa operação no ultimo relabel do discharge
void list_node_height::update_h(int i, int new_h)
{
    // verifica se o nó e o rótulo de altura são válidos
    if (i < 1 || i > num_nodes || new_h < 0 || new_h >= 2*(num_nodes+1)) {
        return;
    }

    // Remoção de i da altura altura atual:
    if (node[i].height != -1)
        remove_h(i);

    // Inserção de i na nova altura:
    node[i].height = new_h;
    node[i].prev = end_link;
    node[i].next = list_height[new_h];

    if (list_height[new_h] != end_link) {       // a lista de altura h não é vazia
        node[list_height[new_h]].prev = i;
    }

    list_height[new_h] = i;
}


int list_node_height::front_h(int h)
{
    return list_height[h];
}


bool list_node_height::empty_h(int h)
{
    return list_height[h] == end_link;
}


void list_node_height::print() {
    for (unsigned int h = 0; h <= 2*num_nodes; h++) {
        std::cout << h << ": ";
        int i = list_height[h];

        while (i != end_link) {
            std::cout << i << " ";
            i = node[i].next;
        }

        std::cout << "\n";
    }
    std::cout << "\n";
}


// int main() {
//     list_node_height nh(6);

//     // teste 1: inicializando todos os nós na mesma altura
//     nh.initialize_all_height(0);

//     nh.print();


//     // teste 2: removendo o primeiro nó de uma lista
//     nh.update_h(1, 2);
//     // teste 3: removendo o nó do meio de uma lista
//     nh.update_h(3, 2);
//     // teste 4: removendo o último nó de uma lista
//     nh.update_h(5, 5);

//     nh.print();


//     // teste 5: removendo o único nó de uma lista
//     nh.update_h(5, 10);

//     nh.print();

//     return 0;
// }