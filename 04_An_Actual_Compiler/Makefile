comp1: cg.c gen.c expr.c interp.c main.c scan.c tree.c
	cc -o comp1 -g  cg.c gen.c expr.c interp.c main.c scan.c tree.c

clean:
	rm -f comp1 *.o *.s out
test: comp1
	./comp1 input01
	cc -o out out.s
	./out
	./comp1 input02
	cc -o out out.s
	./out
