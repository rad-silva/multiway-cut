#ifndef LIST_NODE_HEIGHT
#define LIST_NODE_HEIGHT

// Uma implementação de lista encadeda com base na altura de um nó
// 
//          0:      1:     2:     3:     4:     5:
// node | -1;-1 | -1;4 | -1;-1 | 4;-1 | 1;3 | -1,-1 |
//
// height_list
//  0: | -1 |
//  1: | -1 |
//  2: |  1 |
//  3: |  5 |
//  4: | -1 |
//  5: | -1 |

#include <vector>

typedef struct node_link {
    int height;
    int prev;
    int next;
} node_link;

class list_node_height {
    std::vector<int> list_height;
    std::vector<node_link> node;
    
    const unsigned int num_nodes;
    const int end_link = -1;

public:
    list_node_height(int n);

    void remove_h(int label);
    void update_h(int label, int new_h);
    void initialize_all_height(int h);
    int  front_h(int h);
    bool empty_h(int h);

    void print();
};

#endif