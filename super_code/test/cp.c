#include <syscall.h>

int main(int argc, char **argv)
{
  OpenFileId in;
  OpenFileId out;
  char c;

  if(argc < 2) {
    Write("cp: falta un archivo como argumento\n", 36, ConsoleOutput);
    Exit(1);
  }
  else if(argc > 2) {
    Write("cp: operando extra\n", 19, ConsoleOutput);
    Exit(1);
  }
  else {
    if(((in = Open(argv[0])) != -1) && ((out = Open(argv[1])) != -1)) {
      while(Read(&c, 1, in))
        Write(&c, 1, out);
      Close(in);
      Close(out);
    }
    else {
      Write("cp: no se puede efectuar. No existe el archivo\n", 47, ConsoleOutput);
      Exit(1);
    }
  }
  Exit(0);
}
