CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -I compiler/include
SRC     = $(wildcard compiler/src/*.c)
OBJ     = $(SRC:.c=.o)
TARGET  = tcc

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

compiler/src/%.o: compiler/src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET) $(TARGET).exe

.PHONY: all clean
