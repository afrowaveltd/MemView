
# ==================================================
# MemView - Main Makefile
# ==================================================

# C compiler
CC = gcc

# Preprocessor options and header search paths
CPPFLAGS = -Iinclude

# C compiler options
CFLAGS = -std=c17 -O0 -g -Wall -Wextra -pedantic

# Targets that do not represent actual files
.PHONY: all clean tests

# Default target
all: build/memview

# --------------------------------------------------
# LINKING
# --------------------------------------------------

# Combine both object files into the executable.
# $^ = all prerequisites
# $@ = target
build/memview: build/main.o build/helpers.o
	$(CC) $^ -o $@

# --------------------------------------------------
# COMPILATION
# --------------------------------------------------

# Compile main.c.
# Rebuild if the source or the header changes.
build/main.o: src/main.c include/helpers.h
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Compile helpers.c.
# Rebuild if the implementation or header changes.
build/helpers.o: src/helpers.c include/helpers.h
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# --------------------------------------------------
# CLEAN
# --------------------------------------------------

# Remove generated project files.
clean:
	rm -f build/memview build/main.o build/helpers.o
	rm -f build/test_helpers

# --------------------------------------------------
# TESTS
# --------------------------------------------------

# Build the helper test executable.
build/test_helpers: tests/test_helpers.c build/helpers.o include/helpers.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< build/helpers.o -o $@

# Run all current tests.
test: build/test_helpers
	./build/test_helpers