RM=rm -f
CFLAGS=-g -Wall -std=gnu99
LDFLAGS=-g
LDLIBS=-lglut -lGL -lm -lGLU -lGLEW -lmicrohttpd

OBJS=ase-rover.o server.o shaders.o terrain.o tga.o
all: simulator

simulator: $(OBJS)
	$(CC) $(LDFLAGS) -o simulator $(OBJS) $(LDLIBS)

ase-rover.o: ase-rover.c
server.o: server.h server.c
shaders.o: shaders.h shaders.c
terrain.o: terrain.h terrain.c
tga.o: tga.h tga.c

clean:
	$(RM) $(OBJS) simulator
