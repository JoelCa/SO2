#include <iostream>

//Version de Lista implementada como una lista enlazada
//que actua como pila

struct Node {
  int v;
  Node *next;
};

class Lista {
  Node *head;
  int  cuantos;
public:
  Lista();
  int front();
  void add(int i);
};

Lista::Lista()
{
  cuantos = 0;
  head = NULL;
}

int Lista::front()
{
  return head->v;
}

void Lista::add(int i)
{
  Node *n = new Node;
  n->v = i;
  n->next = head;
  head = n;
  cuantos++;
}

int main()
{
  Lista *l1 = new Lista();
  l1->add(3);
  std::cout << "El tope es " << l1->front() << std::endl;	
  delete l1;
  return 0;
}
