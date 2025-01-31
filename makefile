CC = gcc

CFLAGS = -Wall -Wextra -std=c99 -O2

SRCS = main.c

TARGET = flappy_bird

UNAME_S := $(shell uname -s)

# Linux
ifeq ($(UNAME_S), Linux)
    CFLAGS += `sdl2-config --cflags`
    LDFLAGS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_image
endif

# Macos
ifeq ($(UNAME_S), Darwin)
    CFLAGS += -I/usr/local/include/SDL2 -D_THREAD_SAFE
    LDFLAGS = -L/usr/local/lib -lSDL2 -lSDL2_ttf -lSDL2_image
endif

# Windows
ifeq ($(OS), Windows_NT)
    CFLAGS += -I/mingw64/include/SDL2 -Dmain=SDL_main
    LDFLAGS = -L/mingw64/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image
    TARGET = flappy_bird.exe
endif

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean