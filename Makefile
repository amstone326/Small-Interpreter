all: jc

list.o: list.c list.h
	clang -g -c list.c -Wall

stack.o: stack.c stack.h
	clang -g -c stack.c -Wall

parse.o: parse.c parse.h list.h 
	clang -g -c parse.c -Wall

jc: jc.c parse.o list.o stack.o
	clang -g -Wall -o jc jc.c parse.o list.o stack.o

clean: 
	rm -f *.o jc
