
# Simple Makefile

CC=g++
CFLAGS=-O3 -Wall
LIBOUTPUT=libslasha.a
DBGFLAGS=-DDEBUG -g

C_FILES=SlashA.cpp NR-ran2.cpp 
O_FILES=$(C_FILES:.cpp=.o)

all:
	$(CC) -c $(CFLAGS) $(C_FILES)
	ar rc $(LIBOUTPUT) $(O_FILES)
	ranlib $(LIBOUTPUT)
  
debug:
	$(CC) -c $(DBGFLAGS) $(C_FILES)
	ar rc $(LIBOUTPUT) $(O_FILES)
	ranlib $(LIBOUTPUT)

clean:
	rm -f *.a *.o core a.out *~
