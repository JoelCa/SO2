#include "synchconsole.h"
#include "console.h"
#include "synch.h"

SynchConsole::SynchConsole()
{
  lockList = new Lock* [2];
  lockList[0] = new Lock("console lock for readers");
  lockList[1] = new Lock("console lock for writers");
  
  console = new Console(NULL, NULL, ReadAvail, WriteDone, NULL);
}

SynchConsole::~SynchConsole()
{
  delete console;
  delete lockList[0];
  delete lockList[1];
  delete lockList;
}

void SynchConsole::PutBuff(char *buff, int size)
{
  for(int i=0; i < size; i++) {
    lockList[1]->Acquire();
    console->PutChar(buff[i]);
    writeDone->P();
    lockList[1]->Release();
  }
}

int SynchConsole::GetBuff(char *buff, int size)
{
  for(int i=0; i < size; i++) {
    lockList[0]->Acquire(); 
    readAvail->P();
    buff[i] = console->GetChar();
    lockList[0]->Release();    
  }
  return size;
}

Semaphore *SynchConsole::readAvail = new Semaphore("read avail", 0);
Semaphore *SynchConsole::writeDone = new Semaphore("write done", 0);
void SynchConsole::ReadAvail(void* arg) { readAvail->V(); };
void SynchConsole::WriteDone(void* arg) { writeDone->V(); };
