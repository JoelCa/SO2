#include <syscall.h>

int main(int argc, char **argv)
{
  SpaceId pid;
  int retorno;
  
  pid = Exec("print_a", 0, 0);
  Write("probando", 8, 1);
  while(1)
    Write("b\n", 2, 1);
  retorno = Join(pid);
  Exit(0);
}

