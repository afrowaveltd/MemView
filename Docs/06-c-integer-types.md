# Lesson 06: C Integer Types, Bytes, and Signedness

## Learning objectives

In this lesson we will learn:

- What `sizeof` measures and why `sizeof(char)` is always `1`.
- The difference between a **C byte**, a bit, and a character.
- How character constants differ from string literals in C.
- Why `char`, `signed char`, and `unsigned char` are three distinct types.
- How unsigned integer conversion and arithmetic work.
- Why signed overflow must not be treated as equivalent to unsigned wraparound.
- How to inspect a value in decimal and hexadecimal notation.

The experiments use our actual `src/main.c` and the project-root `Makefile` from [Lesson 05](05-gnu-make.md). We modify the source file locally, run `make`, and inspect its output. These are learning steps; no experimental program is committed automatically.

## 1. What is a byte in C?

A **byte** in C is the size of one `char` object.

By definition:

```c
sizeof(char) == 1
```

This is true even if a byte contains more than eight bits on some implementation.

The macro `CHAR_BIT` from `<limits.h>` tells us how many bits a C byte contains. The C standard requires at least eight bits; common desktop systems use eight.

The `sizeof` operator produces a result of type `size_t`. When printing it with `printf`, we normally use `%zu`.

### 1.1 The first experiment

```c
#include <limits.h>
#include <stdio.h>

int main(void)
{
    printf("Bits per byte: %d\n", CHAR_BIT);

    printf("char:      %zu byte(s)\n", sizeof(char));
    printf("short:     %zu byte(s)\n", sizeof(short));
    printf("int:       %zu byte(s)\n", sizeof(int));
    printf("long:      %zu byte(s)\n", sizeof(long));
    printf("long long: %zu byte(s)\n", sizeof(long long));

    return 0;
}
```

Build and run it from the repository root:

```bash
make
./build/memview
echo $?
```

For our Debian x86-64 GCC build, typical results are:

```text
Bits per byte: 8
char:      1 byte(s)
short:     2 byte(s)
int:       4 byte(s)
long:      8 byte(s)
long long: 8 byte(s)
```

These are **observations about a platform**, not a complete promise of the C language. For example, 64-bit Windows commonly uses a four-byte `long`, whereas our 64-bit Linux toolchain uses an eight-byte `long`.

## 2. A byte is not the same concept as an ASCII character

ASCII encodes 128 values with seven bits. The C language was developed long before Unicode, but the fact that `sizeof(char)` is one is not a special rule derived from ASCII. In C, **the byte is defined through the size of `char`**.

Character encodings tell us how to interpret byte values as text.

UTF-8 uses one to four eight-bit code units to encode a Unicode scalar value. For example, on a UTF-8 system:

| Character | UTF-8 bytes in hex | Bytes |
| --- | --- | --- |
| `A` | `41` | 1 |
| `č` | `C4 8D` | 2 |
| `😀` | `F0 9F 98 80` | 4 |

The bytes themselves are numerical values. How they appear on the screen depends on how the software interprets the sequence.

**For MemView:** A hex viewer must not assume that one byte always represents one complete human-readable character.

## 3. A character constant and a string literal are different

C contains an old historical detail:

- The ordinary character constant `'A'` has type `int`.
- The string literal `"A"` is an array of two `char` elements: `'A'` and the terminating zero `'\0'`.

Try:

```c
#include <limits.h>
#include <stdio.h>

int main(void)
{
    printf("CHAR_BIT: %d\n", CHAR_BIT);
    printf("sizeof(char): %zu\n", sizeof(char));
    printf("sizeof('A'): %zu\n", sizeof('A'));
    printf("sizeof(\"A\"): %zu\n", sizeof("A"));

    return 0;
}
```

On our Debian system, the expected output is:

```text
CHAR_BIT: 8
sizeof(char): 1
sizeof('A'): 4
sizeof("A"): 2
```

**Do not confuse `sizeof('A')` with the physical storage used by a particular compiler.** The operator reports the size of the expression's *type*. An immediate constant in optimized machine code may not require a separate four-byte object in memory at all.

A declaration such as `char letter = 'A';` converts the integer constant to the type of the object being initialized.

### 3.1 ASCII and the numerical value of `'A'`

On ASCII-compatible systems, `'A'` has numerical value `65` (hex `0x41`). The C standard does not require ASCII as the execution character set, so `65` is not universal across all conforming implementations.

## 4. The three character integer types

C has three distinct types:

- `char`
- `signed char`
- `unsigned char`

Each occupies one C byte. Plain `char` is a **distinct type**, and whether it behaves as signed or unsigned is implementation-defined.

We can inspect the ranges through `<limits.h>`:

```c
#include <limits.h>
#include <stdio.h>

int main(void)
{
    printf("Bits per byte: %d\n", CHAR_BIT);

    printf("signed char:   %d to %d\n", SCHAR_MIN, SCHAR_MAX);
    printf("unsigned char: 0 to %u\n", (unsigned int)UCHAR_MAX);
    printf("plain char:    %d to %d\n", CHAR_MIN, CHAR_MAX);

    return 0;
}
```

On the usual Debian x86-64 GCC configuration:

| Type | Minimum | Maximum |
| --- | ---: | ---: |
| `signed char` | −128 | 127 |
| `unsigned char` | 0 | 255 |
| `char` | −128 | 127 (implementation-dependent) |

For an eight-bit `unsigned char`, there are `2^8 = 256` different values.

## 5. Why `unsigned char` wraps around

We now start with the largest representable value and increment it:

```c
#include <limits.h>
#include <stdio.h>

int main(void)
{
    unsigned char number = UCHAR_MAX;

    printf("Before: %u\n", (unsigned int)number);

    number++;

    printf("After:  %u\n", (unsigned int)number);

    return 0;
}
```

Our expected output for `CHAR_BIT == 8` is:

```text
Before: 255
After:  0
```

The C language defines conversion to an unsigned integer type modulo one more than the maximum value of that type.

For our byte:

```text
255 + 1 = 256
256 modulo 256 = 0
```

Binary view:

```text
Before: 11111111   (255)
After:  00000000   (0)
```

This operation does **not** access memory beyond the object. It changes the value stored in the same object.

### 5.1 An important hidden step: integer promotion

The type of `number` is `unsigned char`, but the expression `number++` is not necessarily implemented as an eight-bit arithmetic operation in the abstract C rules.

On our platform, `int` can represent every `unsigned char` value. The value is therefore **promoted to `int`** during the arithmetic. `255 + 1` is representable as an `int`.

The result, 256, is then converted back to `unsigned char` for storage. That unsigned conversion produces zero.

This is a useful example of how **the type of an object** differs from **the type of intermediate arithmetic expressions**.

### 5.2 Small counting experiment

```c
#include <stdio.h>

int main(void)
{
    unsigned char number = 254;

    printf("%u\n", (unsigned int)number);
    number++;
    printf("%u\n", (unsigned int)number);
    number++;
    printf("%u\n", (unsigned int)number);
    number++;
    printf("%u\n", (unsigned int)number);

    return 0;
}
```

Expected output:

```text
254
255
0
1
```

This sequence is completely defined on our platform.

## 6. Signed integer overflow is different

For signed `int`, adding one to its largest value is **not** the same as well-defined unsigned wraparound:

```c
#include <limits.h>

int main(void)
{
    int value = INT_MAX;
    value++;  /* Undefined behavior: do NOT rely on wraparound. */
    return 0;
}
```

This is a **counterexample for explanation, not an exercise to execute**.

A signed arithmetic operation whose mathematical result cannot be represented has **undefined behavior** under C17. A compiler may optimize code under the assumption that such an overflow does not occur. We must not assume that `INT_MAX + 1` safely becomes `INT_MIN`.

A safe increment checks the boundary first:

```c
#include <limits.h>
#include <stdio.h>

int main(void)
{
    int value = INT_MAX;

    if (value < INT_MAX)
    {
        value++;
    }
    else
    {
        printf("Cannot increment: value is already INT_MAX.\n");
    }

    return 0;
}
```

### 6.1 Special nuance: `signed char` and `++`

The statement

```c
signed char number = SCHAR_MAX;
number++;
```

needs more care than the simplified claim "all signed increments overflow".

On our platform, `number` is promoted to `int`, and `127 + 1` is representable as an `int`. The issue is converting 128 **back into signed char**. For an out-of-range conversion to a signed integer type, C17 permits an implementation-defined result or an implementation-defined signal.

This is different from **signed `int` arithmetic overflow**, which is undefined behavior.

**Learning principle:** Before diagnosing an overflow, identify the types of the operands, any promotions, and the type to which the result is assigned.

## 7. From a negative number to a hexadecimal byte

We can now inspect the relationship between signed values and unsigned byte values without invoking undefined behavior.

The conversion from a negative signed integer to an unsigned integer type is defined modulo the unsigned type's range.

```c
#include <limits.h>
#include <stdio.h>

int main(void)
{
    signed char negative = -1;
    unsigned char byte_value = (unsigned char)negative;

    printf("Signed:   %d\n", (int)negative);
    printf("Unsigned: %u\n", (unsigned int)byte_value);
    printf("Hex:      0x%02X\n", (unsigned int)byte_value);

    return 0;
}
```

On our eight-bit-byte system:

```text
Signed:   -1
Unsigned: 255
Hex:      0xFF
```

The expression `(unsigned char)negative` converts the **numeric value** −1 into the corresponding unsigned value (255 on this platform).

The format `%02X` means:

- `%X`: render an unsigned integer in uppercase hexadecimal.
- `2`: request a minimum output width of two characters.
- `0`: pad shorter results on the left using zeroes.

This is the beginning of our hex viewer: understanding how a numeric value can be displayed in different bases.

A useful caution: these are two separate C objects in this example. We are demonstrating a **value conversion**, not using a pointer to reinterpret the exact same bytes in memory. We will investigate raw object representations separately.

## 8. Practice and verification

From the project root, after each source change:

```bash
make
./build/memview
echo $?
```

Before running a sample, write your prediction on paper:

1. What will `sizeof(char)` return on a machine where `CHAR_BIT` is 16?
2. Why is `sizeof("A")` equal to 2 on our computer?
3. Why does `unsigned char` wrap from 255 to zero without writing outside its object?
4. What is the difference between signed arithmetic overflow and conversion to an out-of-range signed integer type?
5. What does `%02X` change: the underlying number or only the way it is printed?

Answers in brief:

1. Always 1, because `sizeof` measures C bytes.
2. One character plus its terminating zero.
3. The result is converted to the unsigned type, using modulo arithmetic.
4. C17 defines them differently: signed arithmetic overflow is undefined; out-of-range conversion to signed has implementation-defined outcomes.
5. Only the display format.

## 9. Where we go next

We now know that a byte has a numerical value and that a type determines how we interpret it.

Next we will study the relationship between **bits**, **signed and unsigned representations**, and **bitwise operations**. These give MemView the tools needed to display binary and hexadecimal views of memory.

The goal is to understand the mechanism before we build the actual viewer.
