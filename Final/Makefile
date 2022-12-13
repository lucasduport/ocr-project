FILES = $(wildcard ../ImageProcessing/*.c ../Solver/*.c ../NeuralNetwork/*.c ../UserInterface/*.c ./*.c)
SRCS = $(filter-out $(wildcard ../ImageProcessing/main*.c ../Solver/main*.c ../NeuralNetwork/main*.c ../UserInterface/main*.c),$(FILES))
OBJS = $(patsubst %.c,%.o,$(SRCS))
DEPS = $(patsubst %.c,%.d,$(SRCS))

CC=clang
CPPFLAGS=
CFLAGS=-g -MMD -std=c99 -O3 -Wextra -Wall `pkg-config --cflags gtk+-3.0 sdl2 SDL2_image` -fsanitize=address -fsanitize=undefined
LDLIBS=-lm -lSDL2 -lSDL2_image `pkg-config --libs gtk+-3.0`

.PHONY: ImageProcessing Solver NeuralNetwork

all: main

ImageProcessing: $(filter ImageProcessing/%,$(OBJS))
	make $@

Solver: $(filter Solver/%,$(OBJS))
	make $@

NeuralNetwork: $(filter NeuralNetwork/%,$(OBJS))
	make $@

UserInterface: $(filter UserInterface/%,$(OBJS))
	make $@

Final: $(filter Final/%,$(OBJS))
	make $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c $< -o $@

main: $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@

clean:
	rm -rf $(OBJS) $(DEPS) main board_* tmpImg/ fastSolved/
