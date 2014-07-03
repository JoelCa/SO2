// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create several threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//
// Parts from Copyright (c) 2007-2009 Universidad de Las Palmas de Gran Canaria
//

#include "copyright.h"
#include "system.h"
#include "synch.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 10 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"name" points to a string with a thread name, just for
//      debugging purposes.
//----------------------------------------------------------------------
void
SimpleThread(void* name)
{
    // Reinterpret arg "name" as a string
    char* threadName = (char*)name;
    
    // If the lines dealing with interrupts are commented,
    // the code will behave incorrectly, because
    // printf execution may cause race conditions.
    for (int num = 0; num < 10; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
	printf("*** thread %s looped %d times\n", threadName, num);
	//interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", threadName);
    //interrupt->SetLevel(oldLevel);
}


//El thread Hilo 0 o main, realiza join a hijo 1, y antes de
//finalizar realiza join a hijo 2.
void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");
    
    char *threadname = new char[128];
    char *threadname2 = new char[128];

    Thread *t1 = new Thread("Hijo 1", true);
    Thread *t2 = new Thread("Hijo 2", true);

    strcpy(threadname, "Hijo 1");
    strcpy(threadname2, "Hijo 2");

    t1->Fork(SimpleThread, (void *) threadname);
    t2->Fork(SimpleThread, (void *) threadname2);

    currentThread->Yield();
    DEBUG('t',"Se realiza Join: Hilo 0, espera a Hijo 1\n");
    t1->Join();
    DEBUG('t',"Finalizó Join: Hilo 0, esperaba a Hijo 1\n");
    //DEBUG('t',"Se realiza Join: Hilo 0, Espera a Hijo 2\n");
    //t2->Join();
    //DEBUG('t',"Finalizó Join: Hilo 0, Espera a Hijo 2\n");
    
    SimpleThread( (void*)"Hilo 0");
}
