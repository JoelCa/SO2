// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "scheduler.h"
#include "system.h"

//Agregado para el ejerc. 4 (plancha 2)
#define NUM_LIST 5

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads to empty.
//----------------------------------------------------------------------

//Modificado para el ejer.4 (Plancha 2)
Scheduler::Scheduler()
{ 
    readyList = new List<Thread *>* [NUM_LIST];
    for(int i = 0; i < NUM_LIST; i++)
        readyList[i] = new List<Thread *>();
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

//Modificado para el ejer.4 (Plancha 2)
Scheduler::~Scheduler()
{ 
    for(int i = 0; i < NUM_LIST; i++)
        delete readyList[i];
    delete [] readyList;
}

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

//Modificado para el ejer.4 (Plancha 2)
void
Scheduler::ReadyToRun (Thread *thread)
{
    int priority;

    thread->setStatus(READY);
    if((priority = thread->GetTimePriority()) == -1) //prioridad -1 indica prioridad nula
      priority = thread->GetPriority(); //ningún thread tiene prioridad nula

    DEBUG('t', "Putting thread %s with priority %d on ready list.\n", thread->getName(), thread->GetPriority());
    (readyList[priority])->Append(thread);
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

//Modificado para el ejer.4 (Plancha 2)
Thread *
Scheduler::FindNextToRun ()
{
    int i;
    //Print();
    for(i=NUM_LIST-1; i>=0 ; i--) {
      if (!readyList[i]->IsEmpty())
        return readyList[i]->Remove();
    }
    return NULL;
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread)
{
    Thread *oldThread = currentThread;
    
#ifdef USER_PROGRAM			// ignore until running user programs 
    if (currentThread->space != NULL) {	// if this thread is a user program,
        currentThread->SaveUserState(); // save the user's CPU registers
	currentThread->space->SaveState();
    }
#endif
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    currentThread = nextThread;		    // switch to the next thread
    currentThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG('t', "Switching from thread \"%s\" to thread \"%s\"\n",
	  oldThread->getName(), nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    SWITCH(oldThread, nextThread);
    
    DEBUG('t', "Now in thread \"%s\"\n", currentThread->getName());

    // If the old thread gave up the processor because it was finishing,
    // we need to delete its carcass.  Note we cannot delete the thread
    // before now (for example, in Thread::Finish()), because up to this
    // point, we were still running on the old thread's stack!
    if (threadToBeDestroyed != NULL) {
      printf("el thread a borrar: %p\n", threadToBeDestroyed);
        delete threadToBeDestroyed;
	threadToBeDestroyed = NULL;
    }
#ifdef USER_PROGRAM
    if (currentThread->space != NULL) {		// if there is an address space
        currentThread->RestoreUserState();     // to restore, do it.
	currentThread->space->RestoreState();
    }
#endif
}

//-------------------------------------------------------------<F12>---------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------

static void
ThreadPrint (Thread* t) {
  t->Print();
}

//Modificado para el ejerc.4 (Plancha 2)
void
Scheduler::Print()
{
    printf("This are all the ready list:\n");
    for(int i = 0; i < NUM_LIST; i++){
        printf("Ready list %d contains:\n", i);
	(readyList[i])->Apply(ThreadPrint);
	printf("\n");
    }
}

//Agregados para el ejerc. 4 (Plancha 2)

//swapPriority cambia de prioridad a thread tWithLock, moviendolo a la cola en la que
//se encuentra currentT, siempre y cuando currenT tenga menor prio. que tWithLock.
void
Scheduler::swapPriority(Thread *currentT, Thread *tWithLock, List<Thread *> *queue)
{
  int maybeHigh,maybeLow;

  if((currentT->GetTimePriority() == -1) && (tWithLock->GetTimePriority() == -1)) {
    maybeHigh = currentT->GetPriority();
    maybeLow = tWithLock->GetPriority();
  }
  else if((currentT->GetTimePriority() == -1) && (tWithLock->GetTimePriority() != -1)) {
    maybeHigh = currentT->GetPriority();
    maybeLow = tWithLock->GetTimePriority();
  }
  else if((currentT->GetTimePriority() != -1) && (tWithLock->GetTimePriority() == -1)) {
    maybeHigh = currentT->GetTimePriority();
    maybeLow = tWithLock->GetPriority();
  }
  else {
    maybeHigh = currentT->GetTimePriority();
    maybeLow = tWithLock->GetTimePriority();
  }  
  if(maybeLow < maybeHigh) {
    currentT->SetTimePriority(maybeLow, true);
    tWithLock->SetTimePriority(maybeHigh, true);
    queue->Append(currentT);

    DEBUG('t',"Thread %s con nueva prioridad temporalmente: %d\nThread %s con nueva prioridad temporalmente: %d\n",
          currentT->getName(), currentT->GetTimePriority(), tWithLock->getName(), tWithLock->GetTimePriority());
    
    if(tWithLock->getStatus() == READY) {
    //borramos a tWithLock en ambas listas por las dudas
    readyList[maybeHigh]->IndexRemove(tWithLock);
    readyList[maybeLow]->IndexRemove(tWithLock);
    //tWithLock intercambia prio. con currentT
    readyList[maybeHigh]->Append(tWithLock);
    }
  }
}

void Scheduler::recoverPriority(List<Thread *> *queue)
{
  Thread *recover;
  int lTPrio;

  while(!queue->IsEmpty()) {
    recover = queue->Remove();
    lTPrio = recover->RemoveTimePriority();
    recover->SetTimePriority(lTPrio, false);
  }
  lTPrio = currentThread->RemoveTimePriority();
  currentThread->SetTimePriority(lTPrio, false);
}

