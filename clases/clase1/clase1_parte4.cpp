#include <iostream>
using namespace std;

//Vemos la Herencia

//"virtual" dice que dependiendo de la clase que sea el objeto
//llama al metodo correcto. O sea, llama al metodo de la clase
//derivada que corresponde.
//Si Base::f() es virtual y Derived la sobreescribe, Derived::f() será la func. llamada


//Lista es una clase abstracta
class Lista {
protected:
  int cuantos;  //es acesible a las clases derivadas, pero NO es publico
public:
  virtual ~Lista() {}; //funcion virtual
  virtual void add(int i) = 0; //funcion virtual pura
  virtual int size() = 0;
  virtual int front() = 0;
  void printFront() {
    cout << front() << endl;
  };
};

class ListaArregloFijo : public Lista {
  int c[100];
public:
  void add(int i) { c[cuantos++] = i; };
  int size() { return cuantos; };
  int front() { return c[0]; };
};

class ListaArregloVariable : public Lista {
  int *c;
public:
  ListaArregloVariable (int s) { cuantos = 0; c = new int[s]; };
  ~ListaArregloVariable() { delete[] c; };
  void add(int i) { c[cuantos++] = i; };
  int size() { return cuantos; };
  int front() { return c[0]; };

};

struct Node {
  int v;
  Node *next;
};

class ListaEnlazada : public Lista {
  Node *head;
public:
  ListaEnlazada() { cuantos = 0; head = NULL; };
  ~ListaEnlazada();
  void add(int i);
  int size() {return cuantos; };
  int front() { return head->v; };
};

ListaEnlazada::~ListaEnlazada()
{
  Node *node;

  for(node=head; node!=NULL;node=node->next) {
    cout << "Borrando nodo" << endl;
    delete node;
  }
}

void ListaEnlazada::add(int i)
{
  Node *n = new Node;
  n->v= i;
  n->next=head;
  head = n;
  cuantos++;
}


//l1,l2 y l3 comparte la misma interfaz
int main()
{
  //pongo que las var. sean de tipo Lista para hacerlas
  //del tipo mas general posible
  Lista *l1 = new ListaArregloVariable(10);
  Lista *l2 = new ListaEnlazada();
  Lista *l3 = new ListaArregloVariable(5);
	
  l1 -> add(3);
  l2 -> add(100);
  cout << l2 -> size() << endl;
  l3 -> add (l1 -> front());
  l3 -> printFront();
  delete l1;
  delete l2;
  delete l3;
}
