// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"

//Agregado para el ejercicio 3 (plancha 4) 
#include "noff.h"

//Agregado para el ejercicio 4 (plancha 4)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define UserStackSize		1024 	// increase this as necessary!


class CoreMap {
 public:
  int virtualPage;
  int sector;
};

class AddrSpace {
 public:
  AddrSpace(OpenFile *executable, char *name);	// Create an address space,
                                                // initializing it with the program
                                                // stored in the file "executable"
  ~AddrSpace();			// De-allocate an address space

  void InitRegisters();		// Initialize user-level CPU registers,
  // before jumping to user code

  void SaveState();			// Save/restore address space-specific
  void RestoreState();		// info on a context switch 

  //Agregados para el ejerc. 4 (plancha 3)
  void setArgs(int argc, char **argv);
  void writeArgs();

  //Agregado para el ejerc. 1 (plancha 4)
  TranslationEntry getEntry(int vpn);
  void putEntry(TranslationEntry e);
  int getNumPages();

  //Agregado para el ejerc. 3 (plancha 4)
  void loadPageEntry(int vpn);

  //Agregado para el ejerc. 4 (plancha 4)
  int index;

  //Agregado para el ejerc. 4 (plancha 4)
  static int addrIndex;


 private:
  TranslationEntry *pageTable;	// Assume linear page table translation
  // for now!
  unsigned int numPages;		// Number of pages in the virtual 
					// address space

  //Agregados para el ejerc. 4 (plancha 3)
  int argc;
  char **argv;

  //Agregado para el ejerc. 3 (plancha 4)
  char *fileName;

  //Agregado para el ejerc. 4 (plancha 4)
  int fileDesc;
  CoreMap *coremap;

  //NoffHeader noffH;
};
#endif // ADDRSPACE_H
