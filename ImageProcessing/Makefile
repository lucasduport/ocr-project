.SILENT: all
.SILENT: run
.SILENT: clean
.SILENT: save
.SILENT: format

all:
	clang -fsanitize=address -fsanitize=undefined -Wall -Wextra -std=c99 -O3 \
	-MMD *.c -c -g `pkg-config --cflags sdl2`
	clang -fsanitize=address *.o -o main -lm -lSDL2 -lSDL2_image

nosan:
	clang -Wall -Wextra -std=c99 -O3 -MMD *.c -c `pkg-config --cflags sdl2`
	clang *.o -o main -lm -lSDL2 -lSDL2_image

run:
	make
	clear
	echo "==================== Running main ====================="
	./main -t Images/image_03.jpeg
	echo "==================== End of output ===================="
	rm main *.d

clean:
	rm -rf *.[do] main *.out
	rm -rf saved_images/
	rm -rf board_*/
	rm -rf COMMIT_MESSAGE
	clear

save:
	make clean
	echo "Enter commit message (optional):"
	read var; \
	echo $$var > COMMIT_MESSAGE
	git add --all
	git status --porcelain >> COMMIT_MESSAGE
	git commit -F COMMIT_MESSAGE
	
format:
	clang-format --style=file -i *.[ch]
