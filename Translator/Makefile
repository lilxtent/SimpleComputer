.PHONY: all clean install uninstall 

CC = gcc
CFLAGS = -Wall -Wextra -MP -MMD -g -c
PROG = sat

DIR = build
DIR2 = bin
FILESC = $(wildcard src/*.c)
OBJECTS = $(subst .c,.o,$(FILESC))

all: $(FILESC) $(PROG)


run:
	./bin/$(PROG)

$(PROG): $(subst src/,build/,$(OBJECTS))
	$(CC) $^ -lm -o bin/$@

build/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

-include build/*.d

clean:  
	rm -rf build/*.o build/*.d build/*.o build/*.d
