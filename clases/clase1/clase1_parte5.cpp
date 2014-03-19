#include <iostream>
using namespace std;

//Vemos la Programación Generica (plantillas)

template <class T1>
class Lista {
  T1 c[10];
  int cuantos;
public:
  Lista() { cuantos = 0; };
  void add(T1 t) { c[cuantos++] = t; };
  T1 front() { return c[0]; };
  Lista<T1> *myself() {return this;};
  void printHead();
};

template <class T1>
void print(Lista<T1>  *l)
{
  cout << l -> front() << endl;
}

template <class T1>
void Lista<T1>::printHead()
{
  print(this);
}

int main()
{
  Lista <int> l1;
  Lista < Lista<int> > l2; //asegurar el espacio: > >
  Lista <int> *a = new Lista <int> ();
	
  l1.add(100);
  l1.printHead();
  cout <<  (l1.myself() == &l1) << endl; //da true

  return 0;
}
