
# Simple Makefile

CC=g++
CFLAGS=-O3 -Wall -I../
LFLAGS=-L../ 
LIBS=-lm
DBGFLAGS=-DDEBUG -g

C_FILES=montecarlo.cpp NR-ran2.cpp 
O_FILES=$(C_FILES:.cc=.o)

all:
	$(CC) -c $(CFLAGS) $(C_FILES)
	$(CC) $(LFLAGS) $(O_FILES) -o montecarlo $(LIBS)

debug:
	$(CC) -c $(DBGFLAGS) $(C_FILES)
	$(CC) $(LFLAGS) $(O_FILES) -o montecarlo $(LIBS)

clean:
	rm -f  *.o core a.out *~ montecarlo

