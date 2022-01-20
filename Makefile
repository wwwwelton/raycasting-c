build:
	gcc -std=c99 ./src/*.c -lSDL2 -o raycast;

run:
	clear && ./raycast;

clean:
	rm raycast;
