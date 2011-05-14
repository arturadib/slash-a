
# Simple Makefile

SLASHPATH=../lib

CC=g++
CFLAGS=-O3 -Wall -I$(SLASHPATH)
LFLAGS=-L$(SLASHPATH)
LIBS=-lm -lslasha
DBGFLAGS=-DDEBUG -g

C_FILES=main.cpp 
O_FILES=$(C_FILES:.cpp=.o)

all:
	$(CC) -c $(CFLAGS) $(C_FILES)
	$(CC) $(LFLAGS) $(O_FILES) -o slash $(LIBS)

debug:
	$(CC) -c $(DBGFLAGS) $(C_FILES)
	$(CC) $(LFLAGS) $(O_FILES) -o slash $(LIBS)

clean:
	rm -f  *.o core a.out *~ slash

