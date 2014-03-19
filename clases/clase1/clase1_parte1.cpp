#include <iostream>

struct Lista {
  int c[100];
  int cuantos;
};

Lista add (Lista l, int i)
{
  l.c [l.cuantos++] = i;
}

Lista init(Lista l)
{
  l.cuantos = 0;
  return l;
}

int size(Lista l)
{
  return l.cuantos;
}

/*
//1º version
int main()
{
  Lista l; se eloja en stack; es una var. automatica
  su vida termina al terminar la funcion main
  add(l,3);
  return 0;
}
*/

//2º version
int main()
{
  Lista l;
  l = init(l);
  l = add(l,3);
	
  return 0;
}
