all: assemble simulate

assemble: assemble.c
	gcc -o assemble assemble.c -lm

simulate: simulate.c
	gcc -o simulate simulate.c -lm

clean:
	rm assemble simulate
