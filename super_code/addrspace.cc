// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"


int AddrSpace::victimIndex = 0;
int AddrSpace::ASID = 0;

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

//Modificado para la plancha 3 y 4
AddrSpace::AddrSpace(OpenFile *executable, char *name)
{
  NoffHeader noffH;
  int size, physPosition;
  unsigned j;
  char swapName[11];
  bool useSwap;
    
  //agregado para el ejercicio 3  de la plancha 4
  fileName = name;

  executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
  if ((noffH.noffMagic != NOFFMAGIC) && 
      (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    SwapHeader(&noffH);
  ASSERT(noffH.noffMagic == NOFFMAGIC);

  // how big is address space?
  size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
    + UserStackSize;	// we need to increase the size
  // to leave room for the stack
  numPages = divRoundUp(size, PageSize);
  size = numPages * PageSize;

  //ASSERT(numPages <= NumPhysPages);		// check we're not trying
  // to run anything too big --
  // at least until we have
  // virtual memory

  DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
        numPages, size);


  //Agregados para el ejerc. 4 (Plancha 4)
  useSwap = false;

  // first, set up the translation 
  pageTable = new TranslationEntry[numPages];
  for (j = 0; j < numPages; j++) {
#ifdef USE_DEMAND_LOADING
    pageTable[j].virtualPage = -1;
#else
    pageTable[j].virtualPage = j;
#endif
    if((physPosition = bitMap->Find()) == -1) {
      useSwap = true;
      limitInMem = j;
      break;
    }
    else {
      pageTable[j].physicalPage = physPosition;
      pageTable[j].valid = true;
      pageTable[j].use = false;
      pageTable[j].dirty = false;
      pageTable[j].readOnly = false;  // if the code segment was entirely on 
      // a separate page, we could set its 
      // pages to be read-only

      //completo el coremap
      coremap[physPosition].vpn = j;
      coremap[physPosition].thread = currentThread;
    }
  }

  printf("el asid: %d\n", ASID);
  sprintf(swapName, "SWAP.%d", ASID);
  ASID++;
  
  //OpenFile* Open(swapName);


  if ((swapDesc = open(swapName, O_CREAT | O_RDWR | O_APPEND)) < 0) {
    printf("error: open\n");
  }
  printf("valor del swapDesc de %s: %d\n", swapName, swapDesc);

  if(useSwap) {
    for(int i = limitInMem; i < numPages; i++) {
      pageTable[i].virtualPage = i;
      pageTable[i].physicalPage = -1;
      pageTable[i].valid = false; //asumo que si "valid" es false, entonces
      //la pagina NO está en memória
      pageTable[i].use = false;
      pageTable[i].dirty = false;
      pageTable[i].readOnly = false;
    }
  }

  // zero out the entire address space, to zero the unitialized data segment 
  // and the stack segment    
  for(j=0;j<numPages;j++) {
    if(pageTable[j].valid) {
      int phys = pageTable[j].physicalPage;
      bzero(&(machine->mainMemory[phys*PageSize]), PageSize);
    }
  }
  // then, copy in the code and data segments into memory

#ifndef USE_DEMAND_LOADING
  if (noffH.code.size > 0) {
    DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
          noffH.code.virtualAddr, noffH.code.size);
    for(int i=0; i < noffH.code.size; i++) {
      char c;
      executable->ReadAt(&c, 1, i + noffH.code.inFileAddr);
      int virt_addr = i + noffH.code.virtualAddr;
      int vpn = virt_addr/PageSize;
      int offset = virt_addr % PageSize;
      if(pageTable[vpn].valid) {
        int phys_page = pageTable[vpn].physicalPage;
        machine->mainMemory[offset+phys_page*PageSize] = c;
      }
      else {
        int phys_sector = vpn; //swapMap[vpn].sector;
        ASSERT(lseek(swapDesc, offset+phys_sector*PageSize, SEEK_SET) >= 0)
          //printf("error: lseek\n");
        write(swapDesc, &c, 1);
      }
    }
  }
  if (noffH.initData.size > 0) {
    DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
          noffH.initData.virtualAddr, noffH.initData.size);
    for(int i=0; i < noffH.initData.size; i++) {
      char c;
      executable->ReadAt(&c, 1, i + noffH.initData.inFileAddr);
      int virt_addr = i + noffH.initData.virtualAddr;
      int vpn = virt_addr/PageSize;
      int offset = virt_addr % PageSize;
      if (pageTable[vpn].valid) {
        int phys_page = pageTable[vpn].physicalPage;
        machine->mainMemory[offset+phys_page*PageSize] = c;
      }
      else {
        int phys_sector = vpn;
        //int phys_sector = swapMap[vpn].sector;
        ASSERT(lseek(swapDesc, offset+phys_sector*PageSize, SEEK_SET) >= 0)
          //printf("error: lseek\n");
        write(swapDesc, &c, 1);        
      }
    }
  }
#endif
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

//Modificado para la plancha 3 y 4
AddrSpace::~AddrSpace()
{
  for(unsigned int i=0; i < numPages; i++) {
    if(pageTable[i].valid)
      bitMap->Clear(pageTable[i].physicalPage);
  }
  delete [] pageTable;
  delete swapBitMap;
  close(swapDesc);
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
  //printf("entro a saveState %p\n", currentThread);
  /*for (int i = 0; i < TLBSize; i++)
    printf("tlb: %d %d\n", i, machine->tlb[i].valid);
    printf("\n");*/
#ifdef USE_TLB
  for (int i = 0; i < TLBSize; i++)
    if(machine->tlb[i].valid)
      currentThread->space->putEntry(machine->tlb[i]);
#endif
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
  //printf("entro a restoreState %p\n", currentThread);
#ifdef USE_TLB
  for (int i = 0; i < TLBSize; i++) {
    machine->tlb[i].physicalPage = -1;
    machine->tlb[i].virtualPage = -1;
    machine->tlb[i].valid = false;
    machine->tlb[i].use = false;
    machine->tlb[i].dirty = false;
    machine->tlb[i].readOnly = false;
  }

  /*for (int i = 0; i < TLBSize; i++)
    printf("tlb: %d %d\n", i, machine->tlb[i].valid);
    printf("\n");*/
#else
  machine->pageTable = pageTable;
  machine->pageTableSize = numPages;
#endif
}

//Agregados para el ejerc. 4 (plancha 3)
void writeStrToUsr(char *str, int usrAddr);

void AddrSpace::setArgs(int argc_, char **argv_)
{
  this->argc = argc_;
  this->argv = argv_;
}

void AddrSpace::writeArgs()
{
  int sp = machine->ReadRegister(StackReg), sp2, i;
  int *argv_usr = new int[argc];
  
  for(i = 0; i < argc; i++) {
    writeStrToUsr(argv[i], sp - (strlen(argv[i]) + 1));
    sp -= strlen(argv[i]) + 1;
    argv_usr[i] = sp;
  }

  sp -= (sp % 4);  
  sp2 = sp - (argc + 1) * 4;

  for(i = 0; i < argc; i++)
    machine->WriteMem(sp2 + i * 4, 4, argv_usr[i]);
  machine->WriteMem(sp2 + i * 4, 4, 0);
  sp = sp2 -16;
  machine->WriteRegister(StackReg,sp);
  machine->WriteRegister(4,argc);
  machine->WriteRegister(5,sp2);

  delete [] argv_usr;
  for(i = 0; i < argc; i++)
    delete [] argv[i];
  delete [] argv;
}

//suponemos que vpn esta el rango correcto
TranslationEntry AddrSpace::getEntry(int vpn)
{
  return pageTable[vpn];
}

void AddrSpace::putEntry(TranslationEntry e)
{
  printf("el vpn: %d\n", e.virtualPage);

  pageTable[e.virtualPage] = e;
    /*
  int vpn = e.virtualPage;
  pageTable[vpn].virtualPage = vpn;
  pageTable[vpn].physicalPage = e.physicalPage;
  pageTable[vpn].valid = e.valid;
  pageTable[vpn].readOnly = e.readOnly;
  pageTable[vpn].use = e.use;
  pageTable[vpn].dirty = e.dirty;
    */
}

int AddrSpace::getNumPages()
{
  return numPages;
}

void AddrSpace::loadPageFromBin(int vpn)
{
  int physPage = bitMap->Find();
  int fileAddr;
  int vaddr;
  int lim, aux = -1;
  NoffHeader noffH;

  OpenFile *executable = fileSystem->Open(fileName);

  executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
  if ((noffH.noffMagic != NOFFMAGIC) && 
      (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    SwapHeader(&noffH);
  ASSERT(noffH.noffMagic == NOFFMAGIC);

  DEBUG('v', "el vpn en loadPageEntry es: %d\n", vpn);
  pageTable[vpn].virtualPage = vpn;
  pageTable[vpn].physicalPage = physPage;
  
  ASSERT(physPage >=0);

  bzero(&(machine->mainMemory[physPage*PageSize]), PageSize);

  vaddr = vpn * PageSize;

  if( (noffH.code.virtualAddr <= vaddr) && (vaddr <= noffH.code.virtualAddr + noffH.code.size) ) {
    fileAddr = noffH.code.inFileAddr + (vaddr - noffH.code.virtualAddr);
    aux = noffH.code.inFileAddr + noffH.code.size - fileAddr; //total de bytes que faltan para llegar al final del segmento de codigo
  }
  else if( (noffH.initData.virtualAddr <= vaddr) && (vaddr <= noffH.initData.virtualAddr + noffH.initData.size) ) {
    fileAddr = noffH.initData.inFileAddr + (vaddr - noffH.initData.virtualAddr);
    aux = noffH.initData.inFileAddr + noffH.initData.size - fileAddr; //total de bytes que faltan para llegar al final del segmento de datos inicializados
  }
  else
    DEBUG('v',"fuera de rango, vaddr: %d\n", vaddr);
  //preguntar
  //else
  //fileAddr = noffH.unInitData.inFileAddr + (vpn - noffH.unInitData.virtualAddr); 

  if(aux != -1) {

    lim = (PageSize <= aux) ? PageSize : aux;

    DEBUG('v', "el inicio del code: %d\n", noffH.code.inFileAddr);
    DEBUG('v', "el size del code: %d\n", noffH.code.size);
    DEBUG('v', "el virtualAddr del code: %d\n", noffH.code.virtualAddr);
    DEBUG('v', "el inicio del data: %d\n", noffH.initData.inFileAddr);
    DEBUG('v', "el size del data: %d\n", noffH.initData.size);
    DEBUG('v', "el virtualAddr del data: %d\n", noffH.initData.virtualAddr);
    DEBUG('v', "el fileAddr: %d\n el vaddr: %d\n el lim: %d\n", fileAddr, vaddr, lim);

    for(int i = 0; i < lim; i++) {
      char c;
      executable->ReadAt(&c, 1, fileAddr + i);
      int virt_addr = i + vaddr;
      int vpn_ = virt_addr/PageSize;
      int phys_page = pageTable[vpn_].physicalPage;
      int offset = virt_addr % PageSize;
      machine->mainMemory[offset+phys_page*PageSize] = c;
    }
  }

  delete executable;
}


//terminar
void AddrSpace::savePageToSwap(int physPage)
{
  int val;
  int vpn = coremap[physPage].vpn;
  int vaddrMem = vpn * PageSize;
  //Sector del swap
  int phys_sector = vpn;
  Thread *victimThread = coremap[physPage].thread;
  ///////////////////////Pasamos la pagina victima a swap
 

  for(int i = 0; i < PageSize; i++) {
    int virt_addr = i + vaddrMem;
    int offset = virt_addr % PageSize;
    char c = machine->mainMemory[offset+physPage*PageSize]; // puedo poner i en vez de offset?

    int swapD = victimThread->space->getSwapDescriptor();
    if((val = lseek(swapD, offset+phys_sector*PageSize, SEEK_SET)) >= 0) {
      printf("error: lseek %d\n", val);

    }
    write(swapD, &c, 1);
  }

  //marcar en el proceso correspondiente que vpn esta en swap
  victimThread->space->toSwap(vpn);
  
  //marco como libre la pagina fisica
  bitMap->Clear(physPage);
}

void AddrSpace::loadPageFromSwap(int vpn, int physPage)
{
  //Sector del swap
  int phys_sector = vpn;
  //int phys_page = getNextPage(pageTable, numPages);
 
  ///////////////////////Pasamos a memoria la pagina que necesitamos
  
  for(int i = 0; i < PageSize; i++) {
    char c;
    ASSERT(lseek(swapDesc, i+phys_sector*PageSize, SEEK_SET) >= 0)
      //printf("error: lseek\n");
    read(swapDesc, &c, 1);

    int virt_addr = i + (vpn * PageSize);
    int offset = virt_addr % PageSize;
    machine->mainMemory[offset+physPage*PageSize] = c;
  }

  //actualizo el coremap
  coremap[physPage].vpn = vpn;
  coremap[physPage].thread = currentThread;
  
  //actualizo la pageTable
  pageTable[vpn].virtualPage = vpn;
  pageTable[vpn].physicalPage = physPage;
  pageTable[vpn].valid = true;

  bitMap->Mark(physPage);
}

void AddrSpace::incIndex()
{
  AddrSpace::victimIndex = (AddrSpace::victimIndex + 1) % NumPhysPages;
}

int AddrSpace::getSwapDescriptor()
{
  return swapDesc;
}

void AddrSpace::toSwap(int vpn)
{
  pageTable[vpn].physicalPage = -1;
  pageTable[vpn].valid = false;
  pageTable[vpn].use = false;
  pageTable[vpn].dirty = false;
  pageTable[vpn].readOnly = false;
}  