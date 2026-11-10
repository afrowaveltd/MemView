# =============================================================
# MemView - Main Makefile
# =============================================================

# C compiler
CC = gcc

# Compiler options
# -std=c17	: use the C17 language standard
# -O0		: disable optimalizations for debugging
# -g		: include debugging information
# -Wall		: enable common compiler warnings
# -Wextra	: enable additional warnings
# -pedantic	: check strict language conformance
CFLAGS = -std=c17 -O0 -g -Wall -Wextra -pedantic

# =============================================================
# TARGETS
# =============================================================

# These targets represent actions, not actual files.
.PHONY: all clean

# Default target: build the application.
all: build/memview




# =============================================================
# BUILD
# =============================================================

# Target: build/memview
# Source: src/main.c
#
# $< = first prerequisite (source file)
# $@ = target (output executable)
#
# Create the build directory if it does not exist.
# Then compile the source file into the executable.
build/memview: src/main.c
	mkdir -p build
	$(CC) $(CFLAGS) $< -o $@

# Remove the generated executable.
# -f prefents errors if the file does not exist.
clean:
	rm -f build/memview
