#include <iostream>

class Lista {
  int *c; //propiedades
  int cuantos;
public:
  void add(int i); //metodos
  int size ();
  int front();
  Lista (int s); //constructor
  ~Lista(); //destructor
};

Lista::Lista(int s)
{
  cuantos = 0;
  //c = malloc(sizeof(int)*s); //esto esta en heap
  c = new int[s]; //aloja en memoria y llama al constructor
  std::cout << "Construyendo lista" << std::endl;
}

Lista::~Lista()
{
  //free(c);
  delete[] c;
  std::cout << "Liberando lista" << std::endl;
}

void Lista::add(int i)
{
  c[cuantos++] = i;
}

int Lista::size()
{
  return cuantos;
}

int Lista::front()
{
  return c[0];
}

Lista nuevaList(int s)
{
  Lista *l1 = new Lista(100);
}


/*
//Version 1
int main()
{
  Lista l(100); //el constructor se llama al crear el objeto
  l.add(3);
  std::cout << "El tope es " << l.front() << std::endl;	
  return 0;
  //el destructor se llama aca, pues l es una var. automatica
}
*/


//Version 2
int main()
{
  int size = 10;
  Lista *l1 = new Lista(size); //llama al constructor
  l1->add(3);
  std::cout << "El tope es " << l1->front() << std::endl;	
  delete l1; //llama al destructor
  //o sea libera al espacio reservado para l1 y luego llama al destructor
  //para liberar el puntero c
  return 0;
}
