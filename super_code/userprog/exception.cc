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
  char *aux = new char('1');
  int i;

  for(i=0; *aux != '\0'; i++, usrAddr++) {
    readMem(usrAddr,1,(int *)aux);
    outStr[i] = *aux;
  }
  outStr[i] = '\0';
}

void readBuffFromUsr(int usrAddr, char *outStr, int byteCount)
{ 
  int *aux = new int;

  for(int i=0; i < byteCount; i++, usrAddr++) {
    readMem(usrAddr, 1, aux);
    outStr[i] = *aux;
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
  char *name = new char[MAXLENGTH];
  
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
  char *buff = new char[MAXLENGTH];
  OpenFile *op;
        
  readBuffFromUsr(vaddr,buff,size);

  if(fd == ConsoleOutput)
    synchConsole->PutBuff(buff, size);
  else if((fd != ConsoleInput) && ((op = currentThread->findOpFile(fd, false)) != NULL))
    op->Write(buff,size);
  else
    DEBUG('u',"error: syscall Write, fd %d invalido\n", fd);

  increasePC();
  //delete [] buff;
}

void syscallRead()
{
  int vaddr = machine->ReadRegister(4);
  int size = machine->ReadRegister(5);
  int fd = machine->ReadRegister(6);
  char *buff = new char[MAXLENGTH];
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
  char *name = new char[MAXLENGTH];
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
  delete [] name;
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
    as = new AddrSpace(op);
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
  else if(which == PageFaultException) { //SOLO por TLB Miss
    static int index = 0;
    unsigned vaddr = machine->ReadRegister(BadVAddrReg); // la direccion virtual que genero el fallo esta en el registro BadVAddrReg
    unsigned vpn = vaddr/PageSize; //ver si esta en rango, y si es de solo lectura o escritura

    //esta bien este if?
    if(vpn < currentThread->space->getNumPages()) {
      if(machine->tlb[index].valid)
        currentThread->space->putEntry(machine->tlb[index]);

      machine->tlb[index] = currentThread->space->getEntry(vpn); // La cargamos en la TLB
      index = (index + 1) % TLBSize; //index es global inicializada en cero
      
      machine->tlb[index].valid = true;
    }
    //printf("hola: %d   %d\n", vpn, currentThread->space->getNumPages());
    // Faltaba:
    //  - Invalidar TODA la TLB cada vez que un proceso comienza a ejecutar, AddrSpace::RestoreState. Echo
    //  - Cuando un proceso deja de ejecutarse o "pisa" una entrada de la TLB, debe actualizar la pageTable con la TLB, AddrSpace::SaveState (sólo las entradas válidas). Echo
  
  }
  else if(which == ReadOnlyException) {
    printf("exepción: ReadOnlyException\n");
  }
  else {
    printf("Unexpected user mode exception %d %d\n", which, type);
    ASSERT(false);
  }
}
