# Lesson 08: Functions, Headers, and Separate Compilation

## Learning objectives

By the end of this lesson, you will understand:

- Why a large `main.c` becomes difficult to maintain.
- How to split a C program into independent source files.
- The difference between a **function declaration** and a **function definition**.
- Why a header file belongs in `include/` and the implementation in `src/`.
- How `#include` and include guards work.
- What `-Iinclude` and the Make variable `CPPFLAGS` do.
- How GNU Make compiles multiple translation units and then links them.
- Why passing a byte to a function does not change the caller's object automatically.
- Why compiling a module against **its own header** is an important habit.

This chapter records our first real MemView module. It follows [Lesson 07: Bitwise Operations, Shifts, and Masks](07-bitwise-operations-and-masks.md).

## 1. The problem: Our first functions should not live in `main.c` forever

We already learned how to test a bit and how to set it using a mask:

```c
value & (1u << position)     /* Test */
value | (1u << position)     /* Set */
```

We now want reusable functions instead of writing those expressions in many places. However, putting every function into `main.c` would make that file harder to read as MemView grows.

A natural first design is:

- `main.c`: the application and examples using the functions.
- `helpers.c`: the implementations of the helper functions.
- `helpers.h`: their publicly visible declarations.

This separates **what a function promises** from **how it works**.

## 2. Our project layout

We chose a separate `include/` directory for headers, keeping source implementations under `src/`:

```text
MemView/
├── Docs/
├── include/
│   └── helpers.h
├── pictures/
├── src/
│   ├── main.c
│   └── helpers.c
├── build/                # ignored generated files
├── Makefile
├── README.md
└── .gitignore
```

The `include/` convention is common, but not mandatory in C. It is a deliberate organizational choice for MemView.

As the project grows, we may use more specific names for individual modules. For now, `helpers` is appropriate for these two small bit functions.

## 3. Declaration versus definition

A function **declaration** tells the compiler its name, return type, and parameter types:

```c
int check_bit(unsigned char value, unsigned int position);
```

The semicolon ends the declaration; there is no function body.

A function **definition** provides the implementation:

```c
int check_bit(unsigned char value, unsigned int position)
{
    return ((unsigned int)value & (1u << position)) != 0u;
}
```

That simplified definition is only an illustration; in our actual program we also validate the position before shifting.

One definition is normally compiled into some translation unit. Multiple source files may include the same function declaration.

If a caller has an incompatible or missing declaration, errors can occur during compilation. If a declaration exists but no required definition can be found, a **linking error** may occur.

This is the same distinction we previously observed using `U get_value` and `T get_value` with `nm`.

## 4. The header: `include/helpers.h`

Create the header:

```bash
mkdir -p include
nano include/helpers.h
```

Its intended contents are:

```c
#ifndef MEMVIEW_HELPERS_H
#define MEMVIEW_HELPERS_H

/*
 * Check whether a specific bit is set.
 * Returns 1 if the bit is set, otherwise 0.
 *
 * Current limitation: an invalid position also returns 0.
 */
int check_bit(unsigned char value, unsigned int position);

/*
 * Set a specific bit to 1.
 * Returns the modified byte.
 * An invalid position leaves the value unchanged.
 */
unsigned char set_bit(unsigned char value, unsigned int position);

#endif
```

### 4.1 Include guards

These three preprocessor directives form an **include guard**:

```c
#ifndef MEMVIEW_HELPERS_H
#define MEMVIEW_HELPERS_H

/* Declarations */

#endif
```

The first time the header is included in a translation unit, the macro `MEMVIEW_HELPERS_H` is not defined, so the preprocessor includes the declarations and defines the macro.

If the header is reached a second time while processing the **same translation unit**, the macro is already defined and the guarded content is skipped.

Include guards do **not** mean a header is used only once across the entire program. Each separately preprocessed `.c` file has its own preprocessing context and needs the declarations it uses.

For our two functions, the header does not need to include other standard headers because the parameter and return types are built-in C types.

### 4.2 Why do function declarations end with a semicolon?

A declaration describes a contract without providing its body. A definition provides a body inside braces and is not followed by a required semicolon.

This distinction will be useful again when we study structures, pointers, and separate libraries.

## 5. The implementation: `src/helpers.c`

A good implementation file should include **its own public header**:

```c
#include <limits.h>
#include "helpers.h"

/*
 * Return 1 if the requested bit is set, otherwise 0.
 */
int check_bit(unsigned char value, unsigned int position)
{
    if (position >= CHAR_BIT)
    {
        return 0;
    }

    return ((unsigned int)value & (1u << position)) != 0u;
}

/*
 * Return a byte with the selected bit set.
 */
unsigned char set_bit(unsigned char value, unsigned int position)
{
    if (position >= CHAR_BIT)
    {
        return value;
    }

    return (unsigned char)(value | (1u << position));
}
```

**A small correction from our first committed version:** Our original `src/helpers.c` included `<stdio.h>`, which it did not use, and omitted `"helpers.h"`. The code compiled, but the better practice is to **remove the unused include and include the module's own header** as shown above. This lets the compiler check that the declarations and definitions have compatible types.

### 5.1 Why do we include `<limits.h>`?

`CHAR_BIT` tells us the number of bits in one C byte, which is also the number of bit positions in an `unsigned char` object.

The positions are numbered from zero, so valid positions are:

```text
0 ... CHAR_BIT - 1
```

On our ordinary Debian x86-64 system this is `0 ... 7`.

An invalid shift count can lead to undefined behavior. We check the position before doing `1u << position`.

Our current handling of invalid input is intentionally simple: `check_bit` returns zero, and `set_bit` leaves the byte unchanged. The downside is that the caller cannot distinguish an invalid position from a valid, clear bit using `check_bit` alone. Later we will discuss APIs that return a separate success/error status.

### 5.2 Why cast back to `unsigned char`?

The expression:

```c
value | (1u << position)
```

is evaluated using integer promotion and the usual arithmetic conversions. It does not necessarily have type `unsigned char`.

The explicit conversion:

```c
(unsigned char)(value | (1u << position))
```

expresses our intent to return one byte after setting the selected bit.

### 5.3 The importance of a module's own header

When `src/helpers.c` includes `"helpers.h"`, the compiler sees the prototypes before the definitions. If we accidentally change a parameter or return type in only one location, the compiler can diagnose the mismatch.

This avoids a class of maintenance errors in which the public contract drifts away from its implementation.

## 6. The caller: `src/main.c`

Our application no longer needs to know how bit masks are constructed:

```c
#include <stdio.h>
#include "helpers.h"

int main(void)
{
    unsigned char value = 0x81;

    printf("Before: 0x%02X\n", (unsigned int)value);
    printf("Bit 5: %d\n", check_bit(value, 5u));

    value = set_bit(value, 5u);

    printf("After:  0x%02X\n", (unsigned int)value);
    printf("Bit 5: %d\n", check_bit(value, 5u));

    return 0;
}
```

Expected output:

```text
Before: 0x81
Bit 5: 0
After:  0xA1
Bit 5: 1
```

The call:

```c
value = set_bit(value, 5u);
```

has two distinct operations:

1. Pass the current *value* to the function.
2. Store the function's return value back into the local object in `main`.

If we simply wrote:

```c
set_bit(value, 5u);
```

the function would still calculate and return a result, but `main` would discard that result. Its original `value` object would not be modified.

**C passes function arguments by value.** In a later lesson we will use pointers to let functions operate on objects through their addresses.

## 7. How does the compiler find the header?

Both source files write:

```c
#include "helpers.h"
```

But `helpers.h` is in `include/`, not beside the `.c` files.

We tell GCC to search that directory using:

```text
-Iinclude
```

Our Makefile uses the conventional variable:

```makefile
# Preprocessor options and header search paths
CPPFLAGS = -Iinclude
```

The letters `CPP` here refer to the **C preprocessor**, not to C++.

Contrast this with:

```makefile
CFLAGS = -std=c17 -O0 -g -Wall -Wextra -pedantic
```

`CFLAGS` contains our C compiler settings; `CPPFLAGS` contains preprocessing options and header search paths.

Because the build command includes both variables, the source can simply name `"helpers.h"` without embedding a relative path such as `"../include/helpers.h"`.

## 8. From two source files to one executable

The important parts of our actual MemView Makefile are:

```makefile
# Compiler and options
CC = gcc
CPPFLAGS = -Iinclude
CFLAGS = -std=c17 -O0 -g -Wall -Wextra -pedantic

# These names are actions rather than output files
.PHONY: all clean

# Default target
all: build/memview

# LINKING: $^ = prerequisites, $@ = target
build/memview: build/main.o build/helpers.o
	$(CC) $^ -o $@

# COMPILATION: $< = first prerequisite
build/main.o: src/main.c include/helpers.h
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/helpers.o: src/helpers.c include/helpers.h
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Remove only outputs produced by this Makefile
clean:
	rm -f build/memview build/main.o build/helpers.o
```

The recipe lines begin with **TAB characters**.

### 8.1 The complete build pipeline

```text
src/main.c  + include/helpers.h
           |
           v
       build/main.o
           |
           +-----------------------+
                                   |
src/helpers.c + include/helpers.h  |
           |                       |
           v                       |
       build/helpers.o             |
           |                       |
           +-----------+-----------+
                       |
                       v
                 build/memview
```

We now see the same compilation/linking distinction in the real project:

- The compiler processes `main.c` and `helpers.c` separately.
- Each translation unit produces its own object file.
- The linker resolves `check_bit` and `set_bit` references from `main.o` using definitions from `helpers.o`.

A header normally supplies declarations; it does **not** automatically link the implementation.

### 8.2 Check the object file symbols

After compiling, try:

```bash
nm build/main.o
nm build/helpers.o
```

Look for lines involving `check_bit` and `set_bit`.

In a normal, unoptimized build, `main.o` should show references like:

```text
U check_bit
U set_bit
```

And `helpers.o` should show definitions like:

```text
T check_bit
T set_bit
```

Exact addresses and other symbols depend on the toolchain and platform.

### 8.3 Test the Make dependencies

From the repository root:

```bash
make clean
make
./build/memview
make
```

The first `make` should compile two sources and link the executable. The next `make`, without changes, should find the output up to date.

Now update the timestamp of **only the implementation**:

```bash
touch -m src/helpers.c
make
```

Make should rebuild `helpers.o` and relink, but leave `main.o` alone.

Next update the timestamp of the **shared header**:

```bash
touch -m include/helpers.h
make
```

Both object files depend on this header, so both should be recompiled before linking.

That is why dependency lists are as important as the compiler commands.

### 8.4 A limitation to remember

Our current Makefile explicitly declares `include/helpers.h` as a prerequisite. This is correct for our tiny project.

Later, as modules include more headers, tracking every transitive header dependency by hand becomes tedious. We will introduce GCC-generated dependency files when there is a real need for them.

## 9. Why are these functions not methods?

If you know C#, the expression:

```text
value.CheckBit(5)
```

may feel natural.

In ordinary C, our helper is a free function:

```c
check_bit(value, 5u)
```

There are no member methods attached to an `unsigned char` object in C.

We can still achieve clear, reusable code by naming functions well and grouping declarations with implementations. Later we will learn about structures, opaque types, and function pointers—but not all at once.

## 10. What we have built

We now have a small but meaningful module:

- A documented public interface under `include/`.
- An implementation under `src/`.
- A client in `main.c`.
- A Makefile that builds the modules separately and links them together.
- Reproducible manual checks for building and incremental recompilation.

Before expanding the API, we should also introduce **automated tests** so that each helper can be checked against multiple inputs without manually reading every printout.

Next we will add `clear_bit()` and `toggle_bit()` to this same module, and then establish our first tests in a dedicated `tests/` directory.

**Principle:** Keep the implementation small, keep the interface explicit, and let the compiler and build system verify as much as possible.
