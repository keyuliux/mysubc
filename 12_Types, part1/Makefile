SRCS= cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c
comp1: $(SRCS)
	cc -o comp1 -g $(SRCS)
clean:
	rm -f comp1 *.o *.s out
test: comp1 input05
	./comp1 input05
	cc -o out out.s
	./out
