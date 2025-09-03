CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Ilib -pthread

SRC = $(wildcard src/*.c) $(wildcard lib/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(wildcard src/*.c)) \
      $(patsubst lib/%.c, build/lib/%.o, $(wildcard lib/*.c))

TARGET = bin/webserver

.PHONY: all clean run dirs

all: dirs $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Build rule for src/
build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build rule for lib/
build/lib/%.o: lib/%.c
	@mkdir -p build/lib
	$(CC) $(CFLAGS) -c $< -o $@

dirs:
	@mkdir -p bin build build/lib

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build bin

# rebuild & run
cbr: clean all run

format:
	find src include -name "*.c" -o -name "*.h" | xargs clang-format -i
