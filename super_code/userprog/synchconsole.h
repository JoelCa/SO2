#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

class Lock;
class Semaphore;
class Console;

class SynchConsole {
public:
  SynchConsole();
  ~SynchConsole();
  void PutBuff(char *buff, int size);
  int GetBuff(char *buff, int size);

private:
  Console *console;
  Lock *lock0, *lock1;
  static Semaphore *readAvail, *writeDone;
  static void ReadAvail(void* arg);
  static void WriteDone(void* arg);
};

#endif
