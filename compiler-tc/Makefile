# TightC Compiler v1.2.3 - Pseudo Self-Host Build System
# This builds the TC-based compiler using the existing C-based compiler

# Stage 0: The existing C compiler
STAGE0 = ../tigc

# Output files
TARGET = tigc123
GEN_C = $(TARGET).c

# Source files (in dependency order)
SRC = src/common.tc \
      src/lexer.tc \
      src/parser.tc \
      src/checker.tc \
      src/emitter.tc \
      src/io.tc \
      src/main.tc

# Build the TC-based compiler using the C-based compiler
all: $(TARGET)

$(GEN_C): $(SRC)
	@echo "=== Stage 0: C compiler building TC compiler ==="
	$(STAGE0) $(SRC) -o $(GEN_C)

$(TARGET): $(GEN_C)
	@echo "=== Compiling generated C code ==="
	gcc $(GEN_C) -o $(TARGET) -std=c11 -Wall -Wextra
	@echo "=== Build complete: $(TARGET) ==="

# Self-host test: TC compiler builds itself
selfhost: $(TARGET)
	@echo "=== Stage 1: TC compiler building itself ==="
	./$(TARGET) $(SRC) -o $(TARGET)-v2.c
	gcc $(TARGET)-v2.c -o $(TARGET)-v2 -std=c11 -Wall -Wextra
	@echo "=== Comparing outputs ==="
	diff $(GEN_C) $(TARGET)-v2.c && echo "SUCCESS: Outputs are identical" || echo "FAILURE: Outputs differ"

# Clean generated files
clean:
	rm -f $(GEN_C) $(TARGET) $(TARGET)-v2.c $(TARGET)-v2

# Test with a sample file
test: $(TARGET)
	./$(TARGET) ../samples/hello.tc -o test_output.c
	@echo "Test compilation complete"

# Install to system (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/tigc

.PHONY: all selfhost clean test install
