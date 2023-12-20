#ifndef PRIORITY_QUEUE_WITH_RESTRICTED_PUSH
#define PRIORITY_QUEUE_WITH_RESTRICTED_PUSH

#include <vector>

class PriorityQueueWithRestrictedPush {
public:
  PriorityQueueWithRestrictedPush() : even_queue(), odd_queue() {};

  // Verifica se a fila está vazia
  bool is_empty() const;

  // Limpa a fila.
  void clear();

  // Insere um novo elemento na fila. Sua prioridade deve ser maior ou igual
  // a maior prioridade presente na fila, menos um. Esta condição é
  // DCHECKed, e violá-lo gera um comportamento de fila incorreto no modo NDEBUG.
  void push(int element, int priority);

  // Retorna o elemento com maior prioridade e o remove da fila.
  int pop();

  void printQueue();

private:
  // Função auxiliar para obter o último elemento de um vetor e removê-lo.
  int pop_back(std::vector<std::pair<int, int> >* queue);

  std::vector<std::pair<int, int> > even_queue;
  std::vector<std::pair<int, int> > odd_queue;
};

#endif