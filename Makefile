CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -I compiler/include
SRC     = $(wildcard compiler/src/*.c)
OBJ     = $(SRC:.c=.o)
TARGET  = tigc

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

compiler/src/%.o: compiler/src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET) $(TARGET).exe

install:
	echo "Installing Tig to C:\tig\""
	@if not exist C:\tig mkdir C:\tig
	copy $(TARGET) C:\tig\
	@echo "Done!"

.PHONY: all clean
