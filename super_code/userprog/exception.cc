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

//Agregado
#include <string.h>


#define MAXLENGTH 32


int indexSC;

//Agregado para la plancha 4
//Deben intentarse a lo sumo 3 veces el acceso a memória
//Una vez para recuperarse de un PageFault, y otra para
//modificar una página que era de solo escritura.

void readMem(int addr, int size, int *value)
{
  if(!machine->ReadMem(addr,size,value))
    if(!machine->ReadMem(addr,size,value))
      machine->ReadMem(addr,size,value);
}

void writeMem(int addr, int size, int value)
{
  if(!machine->WriteMem(addr,size,value))
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

  //Agregado:
  char buffer[50];


  int pid;
  OpenFile *op;
  Thread *t;
  AddrSpace *as;

  readStrFromUsr(vaddr, name);
  if ((op = fileSystem->Open(name)) == NULL) {
    sprintf(buffer,"%s: no se encontró la orden\n", name);
    synchConsole->PutBuff(buffer,strlen(buffer));
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
  int aux[3] = {-1,-1,-1}, victim = indexSC;

  DEBUG('v',"Previo a aplicar el alg. Segunda oportunidad con índice %d\n", victim);
  printCoremap();

  for(int j = 0; j < NumPhysPages; j++) {
    CoreMapEntry entry = coremap[victim];
    if(!entry.use && !entry.dirty ) {
      indexSC = (victim + 1) % NumPhysPages;
      return victim;
    }
    else if( !entry.use && entry.dirty && (aux[0] == -1))
      aux[0] = victim;
    else if( entry.use && !entry.dirty && (aux[1] == -1))
      aux[1] = victim;
    else if ( entry.use && entry.dirty && (aux[2] == -1))
      aux[2] = victim;

    if(entry.use) {
      coremap[victim].use = false; //apagamos el bit de referencia en el coremap
      Thread *t = entry.thread;
      t->space->offReferenceBit(victim); //apagamos el bit de referencia en la PageTable
      if(t == currentThread) {        
        for(int k = 0; k < TLBSize; k++)
          if(machine->tlb[k].physicalPage == victim) {
            machine->tlb[k].use = false; //apagamos el bit de referencia en la TLB
            break;
          }
      }
    }
    victim = (victim + 1) % NumPhysPages;
  }

  for(int i = 0; i < 3; i++) {
    if(aux[i] >= 0) {
      indexSC = (aux[i] + 1) % NumPhysPages;
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
  static int indexTLB = 0;
  unsigned vaddr = machine->ReadRegister(BadVAddrReg);
  unsigned vpn = vaddr/PageSize;
  unsigned vpn2 = vpn;
  int physPage;
  TranslationEntry entry;
  Thread *t;
    
  if((vpn < currentThread->space->getNumPages()) && (vpn >= 0)) {

    entry = currentThread->space->getEntry(vpn);

#ifdef USE_DEMAND_LOADING
    if(entry.virtualPage == -1) { //la página NO está en memoria
      
      entry = currentThread->space->loadPageFromBin(vpn);
      DEBUG('v', "La pagina con vpn %d fue cargada del binario\n", vpn);
    }
#endif

#if defined(USE_TLB) && defined(USE_SWAP)
    TranslationEntry TLBEntry = machine->tlb[indexTLB];
    if(TLBEntry.valid) {
      currentThread->space->putEntry(TLBEntry);
      int physP = TLBEntry.physicalPage;
      coremap[physP].use = TLBEntry.use;
      coremap[physP].dirty = TLBEntry.dirty;
      DEBUG('v', "La pagina con vpn %d fue actualizada antes de pisarla en TLB\n", TLBEntry.virtualPage);
    }
#endif

#if defined(USE_TLB) && !defined(USE_SWAP)
    if(machine->tlb[indexTLB].valid) {
      currentThread->space->putEntry(machine->tlb[indexTLB]);
    }
    machine->tlb[indexTLB] = entry; // cargamos en la TLB
    //DEBUG('v',"TLB Actualizada, vpn: %d,  physPage %d\n", machine->tlb[indexTLB].virtualPage, machine->tlb[indexTLB].physicalPage);
    indexTLB = (indexTLB + 1) % TLBSize;
    DEBUG('v', "La pagina con vpn %d fue cargada en la TLB\n", vpn);

    return ;
#endif

#ifdef USE_SWAP

    if(entry.valid) { //la página está en memoria
      DEBUG('v', "Se carga en TLB %d (PAG. EN MEMORIA): physPage %d, vpn %d, y valid %d\n", indexTLB, entry.physicalPage, entry.virtualPage, entry.valid);
      machine->tlb[indexTLB] = entry; // cargamos en la TLB
      
      //No hace falta modificar la TLB, por que se hace por hardware
      coremap[entry.physicalPage].use = true; //Actualizamos coremap

      indexTLB = (indexTLB + 1) % TLBSize;

      return ;
    }

    physPage = bitMap->Find();

    if(physPage >= 0) {
      entry = currentThread->space->loadPageFromSwap(vpn, physPage);
      
      DEBUG('v', "Hay espacio: página física %d\n", physPage);
    }
    else {
      //Si se quiere utilizar el algoritmo de paginacion que elige segun el nº de pagina fisica a la proxima victima, descomentar la linea de abajo y comentar la 451
      //physPage = currentThread->space->victimIndex;

      physPage = SecondChance();

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
            machine->tlb[i].virtualPage = -1;
            machine->tlb[i].valid = false;
            machine->tlb[i].use = false;
            machine->tlb[i].dirty = false;
            machine->tlb[i].readOnly = false;
            break;
          }
      }
      entry = currentThread->space->loadPageFromSwap(vpn, physPage);
      //Cuando se utiliza el otro algoritmo de paginacion, se debe descomentar la linea de abajo para que se incremente el indice
      //currentThread->space->incIndex();
    }

    //actualizo el coremap
    coremap[physPage].vpn = vpn;
    coremap[physPage].thread = currentThread;
    coremap[physPage].use = true;
    coremap[physPage].dirty = false;
#endif

#ifdef USE_TLB
    DEBUG('v', "Se carga en TLB %d: physPage %d, vpn %d, y valid %d\n\n", indexTLB, entry.physicalPage, entry.virtualPage, entry.valid);

    machine->tlb[indexTLB] = entry;
    indexTLB = (indexTLB + 1) % TLBSize;
#endif

  }
  else {
    printf("error: acceso invalido\n");
    currentThread->Finish();
  }
}


void readOnlyException() 
{
  unsigned vaddr = machine->ReadRegister(BadVAddrReg);
  unsigned vpn = vaddr/PageSize;
  int coremapI, TLBI;

#if defined(USE_TLB) && defined(USE_SWAP)

  for(TLBI = 0; TLBI < TLBSize; TLBI++) 
    if(machine->tlb[TLBI].virtualPage == vpn) {
      coremapI = machine->tlb[TLBI].physicalPage;
      break;
   }
  
  DEBUG('v', "Interrupción ReadOnly: physical page %d\n", coremapI);

  //actualizamos el coremap
  coremap[coremapI].use = true;
  coremap[coremapI].dirty = true; //Cuando se produce el readOnlyException, es cuando
                                  //se marca como dirty la pagina

  //No hace falta modificar los bits used y dirty, por que se hace por hardware
  machine->tlb[TLBI].readOnly = false;
#endif

#if !defined(USE_TLB) && defined(USE_SWAP)

  TranslationEntry e = currentThread->space->getEntry(vpn);

  coremapI = e.physicalPage;

  //actualizamos el coremap
  coremap[coremapI].use = true;
  coremap[coremapI].dirty = true; 

  //No hace falta modificar los bits used y dirty, por que se hace por hardware
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
