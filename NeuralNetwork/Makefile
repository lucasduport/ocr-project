
all:
	clang -std=c99 -O3 -Wextra -Wall -MMD -c *.c
	clang -lm -o NeuralNetwork *.o
	rm -rf *[.do] main

debug:
	clang -v -std=c99 -g -fsanitize=address -ftrapv -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error -O3 -Wextra -Wall -MMD -c *.c
	clang -lm -o NeuralNetwork *.o -fsanitize=address

clean:
	rm -rf *[.do] main

clear:
	rm ./NeuralNetwork
	rm -rf *[.do] main
