#include <syscall.h>

#define MAXLENGTH 128

int main(int argc, char **argv)
{
  OpenFileId op;
  char c, array[MAXLENGTH];
  int i = 0;

  if(argc == 0) {
    while(1) {
      i = 0;
      do {
        Read(array+i, 1, ConsoleInput);
      }
      while((array[i++] != '\n'));
      Write(array, i, ConsoleOutput);
    }
  }
  else {
    while(i < argc) {
      if((op = Open(argv[i++])) == -1) {
        Write("cat: no se puede efectuar. No existe el archivo\n", 48, ConsoleOutput);
        Exit(-1);
      }
      else {
        while(Read(&c, 1, op))
          Write(&c, 1, ConsoleOutput);
        Close(op);
      }
    }
  }
  Exit(0);
}
