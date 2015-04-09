#CC = gcc -DYYDEBUG=1 -I./ -g
CC = gcc -I./ -g

.SUFFIXES:
.SUFFIXES: .c .o
.c.o:
	$(CC) -c $<

leap_parser: y.tab.o lex.yy.o  leap_parser.o
	$(CC) -o leap_parser y.tab.o lex.yy.o leap_parser.o -lfl -lm

lex.tab.o:	lex.yy.c y.tab.h

lex.yy.o y.tab.o:

y.tab.c y.tab.h: algebra.yy
	$(YACC) -d -v algebra.yy
#	$(YACC) -d algebra.yy

lex.yy.c: algebra.l
	$(LEX) -i -d algebra.l
#	$(LEX) algebra.l

clean:
	rm -f *.o  leap_parser y.tab.c y.tab.h
	


