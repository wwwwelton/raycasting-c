build:
	gcc -std=c99 ./src/*.c -lSDL2 -lm -o raycast;

run:
	make && clear && ./raycast && make clean;

clean:
	rm raycast;
