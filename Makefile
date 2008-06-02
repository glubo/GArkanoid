all: garkanoid
CC=g++
COPTS=`sdl-config --cflags` -O2 -g -Wall
CLIBS=`sdl-config --libs` -lGL -lGLU -lSDL_image -lSDL_mixer -g


garkanoid: garkanoid.o
	$(CC) garkanoid.o -o garkanoid $(CLIBS)
	chmod +x garkanoid
	rm garkanoid.o
	mv garkanoid build/
garkanoid.o: garkanoid.cpp
	$(CC) garkanoid.cpp -c -o garkanoid.o $(COPTS) 
