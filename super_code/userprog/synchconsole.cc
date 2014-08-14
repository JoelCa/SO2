#include "synchconsole.h"
#include "console.h"
#include "synch.h"

SynchConsole::SynchConsole()
{
  lock0 = new Lock("console lock for readers");
  lock1 = new Lock("console lock for writers");
  
  console = new Console(NULL, NULL, ReadAvail, WriteDone, NULL);
}

SynchConsole::~SynchConsole()
{
  delete console;
  delete lock0;
  delete lock1;
}

void SynchConsole::PutBuff(char *buff, int size)
{
  for(int i=0; i < size; i++) {
    lock1->Acquire();
    console->PutChar(buff[i]);
    writeDone->P();
    lock1->Release();
  }
}

int SynchConsole::GetBuff(char *buff, int size)
{
  for(int i=0; i < size; i++) {
    lock0->Acquire(); 
    readAvail->P();
    buff[i] = console->GetChar();
    lock0->Release();    
  }
  return size;
}

Semaphore *SynchConsole::readAvail = new Semaphore("read avail", 0);
Semaphore *SynchConsole::writeDone = new Semaphore("write done", 0);
void SynchConsole::ReadAvail(void* arg) { readAvail->V(); };
void SynchConsole::WriteDone(void* arg) { writeDone->V(); };
