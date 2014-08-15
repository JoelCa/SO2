# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as
#GCCDIR = ~/gnu/local/decstation-ultrix/bin/
GCCDIR = `pwd`/mips-dec-ultrix42/mips-
LDFLAGS = -T script -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)


# if you aren't cross-compiling:
#GCCDIR =
#LDFLAGS = -N -T 0
#ASFLAGS =
#CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld

CPP = /lib/cpp
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR) -mips1

all: lib/gcc-lib halt shell matmult sort filetest prueba prueba2 bash print_a cp cat error

lib/gcc-lib:
	ln -s `pwd`/mips-dec-ultrix42/ lib/gcc-lib

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt
	
#Agregado para ejerc. 2 (plancha 3)
prueba.o: prueba.c
	$(CC) $(CFLAGS) -c prueba.c
prueba: prueba.o start.o
	$(LD) $(LDFLAGS) start.o prueba.o -o prueba.coff
	../bin/coff2noff prueba.coff prueba

prueba2.o: prueba2.c
	$(CC) $(CFLAGS) -c prueba2.c
prueba2: prueba2.o start.o
	$(LD) $(LDFLAGS) start.o prueba2.o -o prueba2.coff
	../bin/coff2noff prueba2.coff prueba2

bash.o: bash.c
	$(CC) $(CFLAGS) -c bash.c
bash: bash.o start.o
	$(LD) $(LDFLAGS) start.o bash.o -o bash.coff
	../bin/coff2noff bash.coff bash


print_a.o: print_a.c
	$(CC) $(CFLAGS) -c print_a.c
print_a: print_a.o start.o
	$(LD) $(LDFLAGS) start.o print_a.o -o print_a.coff
	../bin/coff2noff print_a.coff print_a

cp.o: cp.c
	$(CC) $(CFLAGS) -c cp.c
cp: cp.o start.o
	$(LD) $(LDFLAGS) start.o cp.o -o cp.coff
	../bin/coff2noff cp.coff cp

cat.o: cat.c
	$(CC) $(CFLAGS) -c cat.c
cat: cat.o start.o
	$(LD) $(LDFLAGS) start.o cat.o -o cat.coff
	../bin/coff2noff cat.coff cat

error.o: error.c
	$(CC) $(CFLAGS) -c error.c
error: error.o start.o
	$(LD) $(LDFLAGS) start.o error.o -o error.coff
	../bin/coff2noff error.coff error

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o
	$(LD) $(LDFLAGS) start.o shell.o -o shell.coff
	../bin/coff2noff shell.coff shell

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult


filetest.o: filetest.c
	$(CC) $(CFLAGS) -c filetest.c
filetest: filetest.o start.o
	$(LD) $(LDFLAGS) start.o filetest.o -o filetest.coff
	../bin/coff2noff filetest.coff filetest

# Estas reglas sirven para compilar programas simples,
# que consistan en un único fuente.
# Las reglas anteriores para construir los ejecutables
# halt, shell, sort y filetest se podrían suprimir

%: %.c 
	make start.o 
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) start.o $*.o -o $*.coff
	../bin/coff2noff $*.coff $@
	

