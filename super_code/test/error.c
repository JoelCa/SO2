#include "syscall.h"

int
main()
{
  char a[1];

  a[100] = '1';

  Exit(0);
}
