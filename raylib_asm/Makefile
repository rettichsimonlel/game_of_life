game: game.o
	ld -o game game.o -dynamic-linker /usr/lib64/ld-linux-x86-64.so.2 -lc -lraylib
	./game

game.o: game.asm
	fasm game.asm
