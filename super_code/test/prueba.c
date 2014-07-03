#include <syscall.h>

int main(int argc, char **argv) {

  //char buff[4];  
  //Read(buff, 80, Open("prueba.c"));
  Write("bash: error\n", 12, ConsoleOutput);

  Exit(1);
}
