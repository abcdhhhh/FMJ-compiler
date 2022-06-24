objects = main.o assem.o canon.o check.o color.o flowgraph.o fmjAST.o graph.o ig.o liveness.o myutils.o prog.o printtree.o regalloc.o rpi.o symbol.o table.o temp.o tile.o translate.o tree.o types.o util.o y.tab.o lex.yy.o printAST.o

# main
main: $(objects)
	gcc -o main $(objects)
	./main

# run
run: main
	arm-linux-gnueabihf-gcc *.s libsysy.a --static -o source.out
	qemu-arm source.out

# test
# testregalloc: testregalloc.o util.o temp.o rpi.o symbol.o table.o graph.o assem.o flowgraph.o liveness.o ig.o color.o regalloc.o myutils.o tree.o
# 	cc -o testregalloc testregalloc.o util.o temp.o rpi.o symbol.o table.o graph.o assem.o flowgraph.o liveness.o ig.o color.o regalloc.o myutils.o tree.o
# 	./testregalloc 

# lib
table.o: table.h table.c
	cc -w -c table.c

# lex & yacc
y.output: parser.yacc
	yacc -v parser.yacc

y.tab.h: parser.yacc
	yacc -d parser.yacc

y.tab.c: parser.yacc
	yacc -d parser.yacc

lex.yy.c: lexer.l
	lex lexer.l

clean:
	rm -f *.o testregalloc main y.output y.tab.h y.tab.c lex.yy.c *.s source.out
