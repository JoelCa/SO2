#include <syscall.h>

#define MAXARGS 10
#define MAXLENGTH 30

void maybeJoin(int flag, int pid)
{
  if(flag)
    Join(pid);
}

int main()
{
  char c;
  char name[MAXLENGTH];
  char array[MAXARGS][MAXLENGTH];
  char *args[MAXARGS];
  int i, narg, flag = 1;
    
  while(1) {
    Write("--", 2, ConsoleOutput);
    i = 0;
    do {
      Read(&c, 1, ConsoleInput);
    }
    while(c == ' ');
    if(c != '\n') {
      if(c == '&') {
        flag = 0;
        do {
          Read(&c, 1, ConsoleInput);
        }
        while(c == ' ');
        if(c == '\n') {
          Write("bash: error\n", 12, ConsoleOutput);
          continue;
        }
      }
      name[0] = c;
      i++;
      do {
        Read(name+i, 1, ConsoleInput);
      }
      while((name[i++] != ' ') && (name[i-1] != '\n'));

      if(name[i-1] == '\n') {
        name[i-1] = '\0';
        maybeJoin(flag, Exec(name, 0, 0));
      }
      else {
        name[i-1] = '\0';
        narg = 0;
        while(readWord(&narg, array, args))
          ;
        maybeJoin(flag, Exec(name, narg, args));
      }
    }
  }
  Exit(0);
}

int readWord(int *parg, char array[MAXARGS][MAXLENGTH], char *args[MAXARGS])
{
  int i = 0, narg = *parg;
  char c;

  do {
    Read(&c, 1, ConsoleInput);
  }
  while(c == ' ');
  if(c == '\n')
    return 0;
  array[narg][0] = c;
  (*parg)++;
  i++;
  do {
    Read(array[narg]+i, 1, ConsoleInput);
    if(array[narg][i] == '\n') {
      array[narg][i] = '\0';
      args[narg] = array[narg];
      return 0;
    }
  }
  while(array[narg][i++] != ' ');
  array[narg][i-1] = '\0';
  args[narg] = array[narg];
  return 1;
}
