#include "puerto.h"
#include "synch.h"

#include "system.h"

Puerto::Puerto()
{
  lockname = new char* [3];
  l = new Lock* [3];
  for (int i = 0; i < 3; i++) {
    lockname[i] = new char [10];
    sprintf(lockname[i], "Lock %d", i);
    l[i] = new Lock (lockname[i]);
  }
  c = new Condition ("Condition 0", l[0]);
  flag = new bool[2];
  flag[0] = false;
  flag[1] = false;
  message = new int;
}


Puerto::~Puerto()
{
  delete c;
  for(int i = 0; i < 3; i++) {
    delete l[i];
    delete lockname[i];
  }
  delete l;
  delete flag;
  delete message;
}

void Puerto::Send(int m)
{
  l[2]->Acquire();
  l[0]->Acquire();
  DEBUG('s', "Thread %s realiza Send\n", currentThread->getName());
  flag[0] = true;
  while(!flag[1])
    c->Wait();
  *message = m;
  c->Signal();
  flag[0] = false;
  flag[1] = false;
  l[0]->Release();
  DEBUG('s',"Thread %s finaliza Send\n", currentThread->getName());
  l[2]->Release();
}

void Puerto::Receive(int *m)
{
  l[1]->Acquire();
  l[0]->Acquire();
  DEBUG('s', "Thread %s realiza Receive\n", currentThread->getName());
  flag[1] = true;
  if(flag[0])
    c->Signal();
  DEBUG('s',"Thread %s espera la transmición del mensaje\n", currentThread->getName());
  while(flag[1])
    c->Wait();
  *m = *message;
  l[0]->Release();
  DEBUG('s',"Thread %s finaliza Receive\n", currentThread->getName());
  l[1]->Release();
}
