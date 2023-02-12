LIBS = sdl2 opengl
LDFLAGS = `pkg-config --libs $(LIBS)`
LDLIBS = `pkg-config --cflags $(LIBS)`
CFLAGS = -Wall -Wextra -pedantic -lm
all:
	gcc main.c -o main $(LDFLAGS) $(CFLAGS) $(LDLIBS)
