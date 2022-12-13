all:
	clang -Wall -Wextra -std=c99 -O3 -MMD *.c -c
	clang *.o -o solver

clean: 
	rm -rf *.[do] solver *.result
	clear
