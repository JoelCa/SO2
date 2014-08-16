// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "thread.h"

//#define readMem(a,b,c) !(machine->ReadMem(a,b,c)) : (machine->ReadMem(a,b,c)) ? 
//#define writeMem(a,b,c) ASSERT(machine->WriteMem(a,b,c))

#define MAXLENGTH 32


int indexSC;

void readMem(int addr, int size, int *value)
{
  if(!machine->ReadMem(addr,size,value))
    machine->ReadMem(addr,size,value);
}

void writeMem(int addr, int size, int value)
{
  if(!machine->WriteMem(addr,size,value))
    machine->WriteMem(addr,size,value);
}


//Agregados para el ejerc. 1 (plancha 3)
void readStrFromUsr(int usrAddr, char *outStr)
{
  int aux = -1;
  int i;

  for(i=0; aux != '\0'; i++, usrAddr++) {
    readMem(usrAddr,1,&aux);
    outStr[i] = (char)aux;
  }
  outStr[i] = '\0';
}

void readBuffFromUsr(int usrAddr, char *outStr, int byteCount)
{ 
  int aux = -1;

  for(int i=0; i < byteCount; i++, usrAddr++) {
    readMem(usrAddr, 1, &aux);
    outStr[i] = (char)aux;
  }
}

void writeStrToUsr(char *str, int usrAddr)
{
  int i;
  
  for(i=0; str[i] != '\0'; i++, usrAddr++)
    writeMem(usrAddr, 1, str[i]);
  str[i] = '\0';
}

void writeBuffToUsr(char *str, int usrAddr, int byteCount)
{
  for(int i=0; i < byteCount; i++, usrAddr++)
    writeMem(usrAddr, 1, str[i]);
}

void increasePC()
{
  int pc = machine->ReadRegister(PCReg);
  machine->WriteRegister(PrevPCReg, pc);
  pc = machine->ReadRegister(NextPCReg);
  machine->WriteRegister(PCReg, pc);
  pc += 4;
  machine->WriteRegister(NextPCReg, pc);
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

//Agregado para el ejerc. 4 (plancha 3)
void startThread(void* arg)
{
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();
  currentThread->space->writeArgs();
  machine->Run();
  ASSERT(false); 
}

//Agregados para el ejerc. 2 (plancha 3)
void syscallCreate()
{
  int vaddr = machine->ReadRegister(4);
  char name[MAXLENGTH];
  
  readStrFromUsr(vaddr,name);
 
  if(!fileSystem->Create(name, 256))
    DEBUG('u',"error: syscall Create\n");

  increasePC();
}

void syscallWrite()
{
  int vaddr = machine->ReadRegister(4);
  int size = machine->ReadRegister(5);
  int fd = machine->ReadRegister(6);
  char buff[size];
  OpenFile *op;
        
  readBuffFromUsr(vaddr,buff,size);

  if(fd == ConsoleOutput)
    synchConsole->PutBuff(buff, size);
  else if((fd != ConsoleInput) && ((op = currentThread->findOpFile(fd, false)) != NULL))
    op->Write(buff,size);
  else
    DEBUG('u',"error: syscall Write, fd %d invalido\n", fd);

  increasePC();
// delete [] buff;
}

void syscallRead()
{
  int vaddr = machine->ReadRegister(4);
  int size = machine->ReadRegister(5);
  int fd = machine->ReadRegister(6);
  char buff[MAXLENGTH];
  int nbytes;
  OpenFile *op;

  if(fd == ConsoleInput) {
    nbytes = synchConsole->GetBuff(buff, size);
    writeBuffToUsr(buff,vaddr, nbytes);
  }
  else if((fd != ConsoleOutput) && (op = currentThread->findOpFile(fd, false)) != NULL) {
    nbytes = op->Read(buff,size);
    writeBuffToUsr(buff,vaddr, nbytes);
  }
  else {
    DEBUG('u',"error: syscall Read, fd %d invalido\n", fd);
    nbytes = -1;
  }

  machine->WriteRegister(2,nbytes);

  increasePC();
  //delete [] buff;
}

void syscallOpen()
{
  int vaddr = machine->ReadRegister(4);
  int fd;
  char name[MAXLENGTH];
  OpenFile *op;
  
  readStrFromUsr(vaddr,name);
  if((op = fileSystem->Open(name)) != NULL) { 
    DEBUG('u', "Open: se abre \'%s\'\n", name);
    fd = currentThread->addOpFile(op);
  }
  else {
    DEBUG('u',"error: syscall Open, el archivo %s no existe\n", name);
    fd = -1;
  }
  machine->WriteRegister(2, fd);

  increasePC();
  //delete [] name;
}

void syscallClose()
{
  int fd = machine->ReadRegister(4);        
  OpenFile *op;

  if((op = currentThread->findOpFile(fd, true)) != NULL)
    delete op;
  else
    DEBUG('u',"error: syscall Close, fd %d invalido\n", fd);
        
  increasePC();
}

void syscallExit()
{
  int status = machine->ReadRegister(4);

  currentThread->setMsj(status);
  DEBUG('u', "Exit: retorna %d\n", status);
  currentThread->Finish();

  increasePC();
}

void syscallJoin()
{
  int pid = machine->ReadRegister(4);
  int status;
  Thread *t;

  if((t = searchThread(pid)) != NULL) {
    //printf("el thread %p hace Join\n", t);
    t->Join();
    status = t->getMsj();
    DEBUG('u', "Join: el retorno del hijo es %d\n", status);
  }
  else {
    DEBUG('u', "error: syscall Join, pid %d invalido\n", pid);
    status = -1;
  }
  machine->WriteRegister(2, status);

  increasePC();
}

void syscallExec()
{
  int vaddr = machine->ReadRegister(4);
  int argc = machine->ReadRegister(5);
  int argv_addr = machine->ReadRegister(6);
  char **argv = new char*[argc];
  char *name = new char[MAXLENGTH];
  int pid;
  OpenFile *op;
  Thread *t;
  AddrSpace *as;

  readStrFromUsr(vaddr, name);
  if ((op = fileSystem->Open(name)) == NULL) {
    printf("%s: no se encontró la orden\n", name);
    pid = -1;
  }
  else {
    DEBUG('u', "Exec: se abre \'%s\'\n", name);
    for(int i=0; i < argc; i++) {
      int arg;
      readMem(argv_addr + i * 4, 4, &arg);
      argv[i] = new char[MAXLENGTH];
      readStrFromUsr(arg,argv[i]);
      DEBUG('u',"Exec: argumento %d \'%s\'\n", i, argv[i]);
    }
    as = new AddrSpace(op, name);
    t = new Thread(name, true);
    t->space = as;
    t->space->setArgs(argc, argv);
    t->Fork(startThread, NULL);
    pid = addThread(t);
  }

  machine->WriteRegister(2, pid);
  increasePC();
  delete op;
}


void printTLB()
{
  printf("--------->La TLB: inicio\n");
  for(int i = 0; i < TLBSize; i++) {
    printf("physPage: %d, vpn: %d, valid: %d, use: %d, dirty: %d, readOnly: %d\n", machine->tlb[i].physicalPage,
           machine->tlb[i].virtualPage, machine->tlb[i].valid, machine->tlb[i].use, machine->tlb[i].dirty,
           machine->tlb[i].readOnly);
  }
  printf("--------->La TLB: fin\n");
}

#ifdef USE_SWAP
void printCoremap()
{
  printf("--------->La Coremap: inicio\n");
  for(int i = 0; i < NumPhysPages; i++) {
    printf("physPage: %d, vpn: %d, use: %d, dirty: %d, thread: %p\n", i, coremap[i].vpn, coremap[i].use, coremap[i].dirty, coremap[i].thread);
  }
  printf("--------->La Coremap: fin\n");
}

int SecondChance()
{
  int aux[3] = {-1}, i = indexSC, j = 0, k; 
  Thread *t;

  printf("antes:\n");
  printCoremap();

  while(j != NumPhysPages) {
    if( !coremap[i].use && !coremap[i].dirty ) {
      printf("despues:\n");
      printCoremap();
      indexSC = (indexSC + 1) % NumPhysPages;
      return i;
    }
    else if( !coremap[i].use && coremap[i].dirty && (aux[0] == -1))
      aux[0] = i;
    else if( coremap[i].use && !coremap[i].dirty && (aux[1] == -1))
      aux[1] = i;
    else if ( !coremap[i].use && !coremap[i].dirty && (aux[2] == -1))
      aux[2] = i;

    if(coremap[i].use) {
      coremap[i].use = false; //apagamos el bit de referencia
      
      //Esta bien apagar el bit de referencia?
      t = coremap[i].thread;
      t->space->offReferenceBit(i);
      if(t == currentThread) {        
        for(k = 0; k < TLBSize; k++)
          if(machine->tlb[k].physicalPage == i) {
            machine->tlb[k].use = false;
            break;
          }
      }
    }

    i = (i+1) % NumPhysPages;
    j++;
  }

  for(i = 0; i < 3; i++) {
    if(aux[i] >= 0) {
      indexSC = (indexSC + 1) % NumPhysPages;
      printf("despues:\n");
      printCoremap();
      return aux[i];
    }
  }
  ASSERT(false);
}

#endif

//Observación:
//-Si NO se usa SWAP <-> todas las páginas estan en memoria
void pageFaultException()
{
  static int index = 0;
  unsigned vaddr = machine->ReadRegister(BadVAddrReg); // la direccion virtual que genero el fallo esta en el registro BadVAddrReg
  unsigned vpn = vaddr/PageSize; //ver si esta en rango, y si es de solo lectura o escritura
  unsigned vpn2 = vpn;
  int physPage;
  TranslationEntry entry;
  Thread *t;
  
  //DEBUG('v', "\nLos datos son vpn %d, vaddr %d, index %d, num pages %d\n", vpn, vaddr, index, currentThread->space->getNumPages());

  //printTLB();
  
  if((vpn < currentThread->space->getNumPages()) && (vpn >= 0)) {

    entry = currentThread->space->getEntry(vpn);

#ifdef USE_DEMAND_LOADING
    if(entry.virtualPage == -1) { //la página NO está en memoria
      
      entry = currentThread->space->loadPageFromBin(vpn);
      DEBUG('v', "La pagina con vpn %d fue cargada del binario\n", vpn);
    }
#endif

#if defined(USE_TLB) && defined(USE_SWAP)
    TranslationEntry TLBEntry = machine->tlb[index];
    if(TLBEntry.valid) {
      currentThread->space->putEntry(TLBEntry);
      int physP = TLBEntry.physicalPage;
      coremap[physP].use = TLBEntry.use;
      coremap[physP].dirty = TLBEntry.dirty;
      DEBUG('v', "La pagina con vpn %d fue actualizada antes de pisarla en TLB\n", TLBEntry.virtualPage);
    }
#endif



#if defined(USE_TLB) && !defined(USE_SWAP)
    machine->tlb[index] = entry; // cargamos en la TLB
    //DEBUG('v',"TLB Actualizada, vpn: %d,  physPage %d\n", machine->tlb[index].virtualPage, machine->tlb[index].physicalPage);
    index = (index + 1) % TLBSize;
    DEBUG('v', "La pagina con vpn %d fue cargada en la TLB\n", vpn);

    return ;
#endif

#ifdef USE_SWAP

    if(entry.valid) { //la página está en memoria
      DEBUG('v', "Se carga en TLB %d (PAG. EN MEMORIA): physPage %d, vpn %d, y valid %d\n", index, entry.physicalPage, entry.virtualPage, entry.valid);
      machine->tlb[index] = entry; // cargamos en la TLB
      
      coremap[entry.physicalPage].use = true;

      index = (index + 1) % TLBSize;

      return ;
    }

    //bitMap->Print();
    physPage = bitMap->Find();

    if(physPage >= 0) {
      entry = currentThread->space->loadPageFromSwap(vpn, physPage);
      
      DEBUG('v', "Hay espacio: página física %d\n", physPage);
    }
    else {
      //physPage = currentThread->space->victimIndex;

      physPage = SecondChance();

      //bitMap->Print();
      DEBUG('v', "NO hay espacio: la página victima es %d\n", physPage);

      t = currentThread->space->savePageToSwap(physPage);
      //actualizo el coremap
      coremap[physPage].vpn = -1;
      coremap[physPage].thread = NULL;
      coremap[physPage].use = false;
      coremap[physPage].dirty = false;

      if(t == currentThread) {
        //Actualizamos la TLB
        for(int i =0; i < TLBSize; i++)
          if(machine->tlb[i].physicalPage == physPage) {
            DEBUG('v', "TLB actualizada al elegir página víctima\n");
            machine->tlb[i].physicalPage = -1;
            machine->tlb[i].valid = false;
            machine->tlb[i].use = false;
            machine->tlb[i].dirty = false;
            machine->tlb[i].readOnly = false;
            break;
          }
      }
      entry = currentThread->space->loadPageFromSwap(vpn, physPage);
      //currentThread->space->incIndex();
    }

    //actualizo el coremap
    coremap[physPage].vpn = vpn;
    coremap[physPage].thread = currentThread;
    coremap[physPage].use = true;
      
#endif

#ifdef USE_TLB
    DEBUG('v', "Se carga en TLB %d: physPage %d, vpn %d, y valid %d\n\n", index, entry.physicalPage, entry.virtualPage, entry.valid);

    machine->tlb[index] = entry;
    index = (index + 1) % TLBSize; //index es global inicializada en cero      
#endif

  }
  else {
    printf("error: acceso invalido\n");
    currentThread->Finish();
  }
}


void readOnlyException() 
{

  unsigned vaddr = machine->ReadRegister(BadVAddrReg); // la direccion virtual que genero el fallo esta en el registro BadVAddrReg
  unsigned vpn = vaddr/PageSize; //ver si esta en rango, y si es de solo lectura o escritura
  int indexCoremap, indexTLB;
  
  DEBUG('v', "Interrupción ReadOnly: vpn %d\n", vpn);


#if defined(USE_TLB) && defined(USE_SWAP)

  for(indexTLB = 0; indexTLB < TLBSize; indexTLB++) 
    if(machine->tlb[indexTLB].virtualPage == vpn) {
      indexCoremap = machine->tlb[indexTLB].physicalPage;
      break;
   }
  
  DEBUG('v', "Interrupción ReadOnly: physical page %d\n", indexCoremap);

  //actualizamos el coremap
  coremap[indexCoremap].use = true;
  coremap[indexCoremap].dirty = true; //Cuando se produce el readOnlyException, es cuando
                               //se marca como dirty la pagina

  //actualizamos la tlb
  machine->tlb[indexTLB].use = true;
  machine->tlb[indexTLB].dirty = true;
  machine->tlb[indexTLB].readOnly = false;
#endif

/*
#ifndef USE_SWAP

  printf("error: acceso invalido\n");
  currentThread->Finish();

#endif
*/

#if !defined(USE_TLB) && defined(USE_SWAP)

  TranslationEntry e = currentThread->space->getEntry(vpn);

  indexCoremap = e.physicalPage;

  //actualizamos el coremap
  coremap[indexCoremap].use = true;
  coremap[indexCoremap].dirty = true; //en el primer intento de escritura se produce el readOnlyException, y es cuando
                               //se marca como dirty la pagina, en el segundo intento se modifica la pagina

  e.use = true;
  e.dirty = true;
  e.readOnly = false;
  //actualizamos la pageTable
  currentThread->space->putEntry(e);
#endif

}

void
ExceptionHandler(ExceptionType which)
{
  int type = machine->ReadRegister(2);

  if(which == SyscallException) {
    switch(type) {
      case SC_Halt:
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
        break;
        
      case SC_Create:
        syscallCreate();
        break;

      case SC_Write:
        syscallWrite();
        break;
            	
      case SC_Read:
        syscallRead();
        break;
        
      case SC_Open:
        syscallOpen();
        break;
        
      case SC_Close:
        syscallClose();
        break;
        
      case SC_Exit:
        syscallExit();
        break;

      case SC_Join:
        syscallJoin();
        break;
        
      case SC_Exec:
        syscallExec();
        break;

      default:
        break;
    }
  }
  else if(which == PageFaultException) {
    pageFaultException();
  }
  else if(which == ReadOnlyException) {
    readOnlyException();
  }
  else {
    printf("Unexpected user mode exception %d %d\n", which, type);
    ASSERT(false);
  }
}
