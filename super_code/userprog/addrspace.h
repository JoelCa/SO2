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
#include "openfile.h"
#include "bitmap.h"

#define UserStackSize		1024 	// increase this as necessary!


class Thread;

class SwapMap {
 public:
  int virtualPage;
  int sector;
  bool valid;
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
  TranslationEntry loadPageFromBin(int vpn);

  //Agregado para el ejerc. 4 (plancha 4)
  TranslationEntry loadPageFromSwap(int vpn, int physPage);
  Thread *savePageToSwap(int vpn);
  //void setASID(int value) {asid = value; };
  OpenFile *getSwap();
  void toSwap(int vpn);

  void bitsOff();
  void offReferenceBit(int physPage);


  void print();

  static int SWAPID;
  static int victimIndex;
  static void incIndex();

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
  char swapName[11];
  OpenFile *swap;
  int limitInMem;
  
};
#endif // ADDRSPACE_H
