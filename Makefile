CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -pthread
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
TARGET = bin/webserver

.PHONY: all clean run dirs

all: dirs $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

dirs:
	@mkdir -p bin build

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build bin

# rebuild & run
rbr: clean all run
