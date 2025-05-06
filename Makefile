CC = gcc
CFLAGS = -Wall -Werror -std=c99
LDFLAGS =

ifeq ($(OS),Windows_NT)
	EXECUTABLE = tinybft_demo.exe
else
	EXECUTABLE = tinybft_demo
endif

SOURCES = tinybft_demo.c memory_layout.c
HEADERS = memory_layout.h

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -f $(EXECUTABLE)
