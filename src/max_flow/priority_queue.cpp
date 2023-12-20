#include <iostream>
#include <vector>

#include "priority_queue.hpp"

bool PriorityQueueWithRestrictedPush::is_empty() const
{
  return even_queue.empty() && odd_queue.empty();
}

void PriorityQueueWithRestrictedPush::clear()
{
  even_queue.clear();
  odd_queue.clear();
}

void PriorityQueueWithRestrictedPush::push(int element, int priority)
{
  bool i = (even_queue.empty() || priority >= even_queue.back().second - 1);
  bool j = (odd_queue.empty() || priority >= odd_queue.back().second - 1);
  if (!(i && j)) return;

  if (priority & 1) {
    if(!(odd_queue.empty() || priority >= odd_queue.back().second)) return;
    odd_queue.push_back(std::make_pair(element, priority));
    
  } else {
    if(!(even_queue.empty() || priority >= even_queue.back().second)) return;
    even_queue.push_back(std::make_pair(element, priority));
  }
}

int PriorityQueueWithRestrictedPush::pop()
{
  if(is_empty()) return -1;

  if (even_queue.empty()) return pop_back(&odd_queue);
  if (odd_queue.empty()) return pop_back(&even_queue);

  if (odd_queue.back().second > even_queue.back().second) {
    return pop_back(&odd_queue);
  } else {
    return pop_back(&even_queue);
  }
}

int PriorityQueueWithRestrictedPush::pop_back(std::vector<std::pair<int,int> > *queue)
{
  if (queue->empty()) return -1;
  int element = queue->back().first;
  queue->pop_back();
  return element;
}

void PriorityQueueWithRestrictedPush::printQueue() {
  std::cout << "Even Queue: ";
  for (const auto& pair : even_queue) {
    std::cout << "(" << pair.first << ", " << pair.second << ") ";
  }
  std::cout << "\n";

  std::cout << "Odd Queue: ";
  for (const auto& pair : odd_queue) {
    std::cout << "(" << pair.first << ", " << pair.second << ") ";
  }
  std::cout << "\n\n";
}



// int main() {
//   PriorityQueueWithRestrictedPush pq;

//   // pq.push(elemento, prioridade)

//   // teste 1: inserção de elemento na fila impar vazia
//   pq.push(10, 3);

//   // teste 2: inserção de elemento na fila impar quando ja tem um elemento
//   pq.push(20, 5);

//   // teste 3: inserção de elemento na fila par vazia
//   pq.push(15, 4);

//   // teste 4: inserção de elemento com prioridade 1 unidade menor que a maior prioridade presente na fila
//   pq.push(3, 4);

//   // teste 5: inserção de elemento na fila com prioridade igual a maior presente na fila
//   pq.push(5, 5);

//   // teste 6: tentativa de inserção de elemento com pririodade duas unidades menor que a maior presente na fila
//   pq.push(80, 3);

//   pq.printQueue();

//   while (!pq.is_empty()) {
//     int element = pq.pop();
//     std::cout << "Popped element: " << element << std::endl;
//   } std::cout << "\n";

//   pq.printQueue();

//   return 0;
// }