# Lesson 13: Nested Loops and Our First Multi-Byte Display

> **Purpose:** Bring together arrays, loops, formatting, and our tested `check_bit()` helper. We are not building a full memory viewer yet: we are learning to display a known, valid sequence of bytes correctly.

## Learning objectives

By the end of this lesson, students can:

- Explain what a **nested loop** is and when it is useful.
- Follow the execution of outer and inner loops without losing track of their counters.
- Print the contents of an array in both hexadecimal and binary.
- Distinguish a **byte offset**, an **array index**, and a **bit position**.
- Use `size_t` and `sizeof` to avoid hard-coded array lengths.
- Print bits in the conventional MSB-to-LSB order without an unsigned countdown bug.
- Understand why our helper functions should remain independent of presentation.
- Recognize the boundary between a display of known values and an actual memory-reading tool.

**Prerequisites:** [Lesson 07: Bitwise Operations](07-bitwise-operations-and-masks.md), [Lesson 11: Loops and Binary Display](11-loops-and-binary-output.md), and [Lesson 12: Arrays, Memory Layout, and sizeof](12-arrays-memory-layout-and-sizeof.md).

## 1. The problem: one byte is no longer enough

We have already displayed the bits of a single `unsigned char` using our function:

```c
int check_bit(unsigned char value, unsigned int position);
```

Now imagine four bytes:

```c
unsigned char bytes[] = {
    0x96u,
    0xA5u,
    0xFFu,
    0x00u
};
```

For these four values, our goal is:

```text
Offset  Hex  Binary
0000    96   10010110
0001    A5   10100101
0002    FF   11111111
0003    00   00000000
```

The **outer loop** selects which byte to display. The **inner loop** selects which bit *within that byte* to print.

This separation is central to our MemView design: a byte's place in a buffer is different from the significance of its individual bits.

## 2. What is a nested loop?

A nested loop is simply a loop inside another loop.

Begin with a simple example in C:

```c
#include <stdio.h>

int main(void)
{
    for (unsigned int row = 0u; row < 2u; ++row)
    {
        for (unsigned int column = 0u; column < 3u; ++column)
        {
            printf("(%u,%u) ", row, column);
        }

        putchar('\n');
    }

    return 0;
}
```

Expected output:

```text
(0,0) (0,1) (0,2)
(1,0) (1,1) (1,2)
```

The outer loop runs twice. **For each outer iteration**, the inner loop starts over and runs three times.

A small trace makes this easier to see:

| Outer counter `row` | Inner counter `column` | Operation |
| ---: | ---: | --- |
| 0 | 0 | Print (0,0) |
| 0 | 1 | Print (0,1) |
| 0 | 2 | Print (0,2) |
| 1 | 0 | Print (1,0) |
| 1 | 1 | Print (1,1) |
| 1 | 2 | Print (1,2) |

There are 2 × 3 = 6 executions of the inner body.

The inner counter is **declared inside its own `for` loop**, so each time the outer body starts another inner loop, it is initialized again.

### A useful mental model

Think of a book:

- Outer loop: choose a page.
- Inner loop: read the lines on that page.
- When you turn the page, you start reading its lines from the beginning.

For MemView:

- Outer loop: choose the byte.
- Inner loop: inspect that byte's bits.

## 3. Before writing the binary output, print the hex values

We should build the solution in understandable stages.

```c
#include <stddef.h>
#include <stdio.h>

int main(void)
{
    unsigned char bytes[] = { 0x96u, 0xA5u, 0xFFu, 0x00u };
    size_t count = sizeof bytes / sizeof bytes[0];

    for (size_t index = 0; index < count; ++index)
    {
        printf("%04zu    %02X\n",
               index,
               (unsigned int)bytes[index]);
    }

    return 0;
}
```

Expected output:

```text
0000    96
0001    A5
0002    FF
0003    00
```

The index is printed in **decimal**, padded to at least four digits using `%04zu`. This is a useful *relative element offset* in our one-byte-element array, not an absolute process address and not a hexadecimal offset.

The value is printed in **hexadecimal** using `%02X`. We convert the argument to `unsigned int` so its type explicitly matches the format.

On our Debian system, `CHAR_BIT` is eight, so one byte can be shown with exactly two hexadecimal digits. On an implementation with a wider C byte, `%02X` is still valid but two digits would not always be enough; the width is a **minimum**, not a maximum.

Why is the byte at index 2 called offset 2? Because every `unsigned char` is one C byte. If we later use an array of a larger element type, the byte offset is not necessarily the same as the element index.

## 4. Add the inner loop for each byte's bits

Now reuse the already-tested `check_bit()` helper:

```c
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

#include "helpers.h"

int main(void)
{
    unsigned char bytes[] = {
        0x96u,
        0xA5u,
        0xFFu,
        0x00u
    };

    size_t count = sizeof bytes / sizeof bytes[0];

    puts("Offset  Hex  Binary");

    for (size_t index = 0; index < count; ++index)
    {
        printf("%04zu    %02X   ",
               index,
               (unsigned int)bytes[index]);

        for (unsigned int remaining = (unsigned int)CHAR_BIT;
             remaining > 0u;
             --remaining)
        {
            unsigned int position = remaining - 1u;

            printf("%d", check_bit(bytes[index], position));
        }

        putchar('\n');
    }

    return 0;
}
```

Build as usual from the repository root:

```bash
make
./build/memview
```

Expected output on our current eight-bit-byte Debian machine:

```text
Offset  Hex  Binary
0000    96   10010110
0001    A5   10100101
0002    FF   11111111
0003    00   00000000
```

**Notice:** We use the same project Makefile and the same `helpers.o` implementation. We did not need to write new bitwise logic inside `main.c`.

## 5. Follow the first row carefully

The outer loop begins with:

```c
index = 0
```

Therefore the current byte is:

```c
bytes[0] == 0x96u
```

The hexadecimal output prints:

```text
0000    96
```

Before the newline, the inner loop starts at `remaining = CHAR_BIT`. On our current machine this is 8, and the loop computes:

```text
remaining:  8 7 6 5 4 3 2 1
position:   7 6 5 4 3 2 1 0
bit:        1 0 0 1 0 1 1 0
```

The result of the complete first row is:

```text
0000    96   10010110
```

The inner loop ends when `remaining` becomes zero, and then `putchar('\n')` moves to the next output line.

Only then does the outer loop increment `index` to 1. The inner loop will begin **again at the highest bit** for `bytes[1]`.

### 5.1 Why not count down directly from seven with an unsigned counter?

This tempting version is wrong:

```c
/* WRONG: condition never becomes false for unsigned int */
for (unsigned int position = 7u; position >= 0u; --position)
{
    /* ... */
}
```

`position >= 0u` is always true. When `position` decreases below zero, it wraps to `UINT_MAX`.

Instead, our loop checks `remaining > 0u` and only then computes `remaining - 1u`. This cleanly includes bit zero and terminates.

A signed `int` counter would also work for the simple eight-bit demonstration. The unsigned remaining-count idiom is useful to know because it generalizes safely to countdowns.

## 6. Three different numbers in the same line

Take the row:

```text
0002    FF   11111111
```

There are three independent concepts here:

1. **Offset/index 2:** choose `bytes[2]`, the third element in our array.
2. **Hex value FF:** this byte has numerical value 255 on our eight-bit-byte system.
3. **Bit positions 7 ... 0:** display its eight bits in descending significance.

**The displayed sequence is not the physical order of bits in hardware.** It is a deliberate readable representation of the value.

Likewise, **endianness** is about byte order within multi-byte objects, not about which bit we choose to print first.

When we later display a multi-byte integer, we will need to distinguish its numeric value, its individual memory bytes, and the order in which those bytes appear.

## 7. What is the role of `sizeof` here?

We calculate:

```c
size_t count = sizeof bytes / sizeof bytes[0];
```

That expression correctly computes the length because `bytes` is a **real array with a complete type visible in `main()`**.

Do not copy this formula into a function that receives `unsigned char data[]` and assume it will still know the original length. In a parameter declaration, the array syntax is adjusted to a pointer; the original number of elements is not carried automatically.

Eventually, we will build an API resembling:

```c
void display_bytes(const unsigned char data[], size_t count);
```

At that stage, the caller must provide the element count explicitly. We will examine array-to-pointer conversion and `const` before implementing such an interface.

## 8. Why not put `printf` inside our helper module?

Our `helpers.c` currently performs bit calculations:

```c
check_bit()
set_bit()
clear_bit()
toggle_bit()
```

That is a coherent responsibility.

If the helper module began printing headers, spaces, offsets, and newlines, it would be mixing **data operations** with **presentation**.

For now the example output belongs in `main.c`, while our common bit calculations remain in `helpers.c`.

Later we can introduce a dedicated display or formatting module. It can be tested independently when we learn how to write a formatted result into a caller-supplied buffer.

## 9. Practice experiments

Experiment A: change the data:

```c
unsigned char bytes[] = { 0x00u, 0x01u, 0x80u, 0xFFu };
```

Predict the full binary output of all four lines before compiling.

Experiment B: try an array with only **one** element, then an array with **six**. The `sizeof`-based count will adapt without changing the loop bound.

Experiment C: modify the *display order* by looping positions upward:

```c
for (unsigned int position = 0u;
     position < (unsigned int)CHAR_BIT;
     ++position)
{
    printf("%d", check_bit(bytes[index], position));
}
```

That will print each byte's bits **LSB-first**. The underlying values in the array are unchanged.

Experiment D: compare the number of times the outer loop runs with the total number of calls to `check_bit()`.

On our ordinary eight-bit machine with four bytes:

```text
Outer iterations:       4
Inner iterations/byte:  8
Total bit checks:      32
```

This is our first glimpse of the amount of work performed by nested loops. As we progress, we will talk about complexity, but first make the execution model clear.

## 10. Questions for students

1. Why does the inner loop start over for every byte?
2. How many times will the inner body execute for five bytes on a machine with `CHAR_BIT == 8`?
3. What exactly does the left-hand column `0002` represent?
4. Does `%02X` guarantee exactly two digits for every conceivable C byte width?
5. Why is the element count derived from `sizeof` rather than hard-coded?
6. Why do we use `remaining > 0u` in the descending bit loop?
7. Does displaying `bytes[index]` as binary change the value stored in the array?
8. Can the same helper functions be used with a different output format?
9. How would we change the printed *order* of bits without touching the actual bytes?
10. What must a future function know besides a pointer to the start of a byte sequence to avoid accessing beyond its end?

## Summary

- The **outer loop** selects an array element.
- The **inner loop** selects a bit within that element.
- Our tested `check_bit()` handles the bit-mask calculation.
- Formatting and data operations remain separate concerns.
- A correct loop boundary matters both for arrays and for unsigned bit positions.
- We now have a small, deterministic byte display: the first recognizable piece of MemView's future hex/binary output.

**Next:** Arrays as function arguments, pointers, `&` (address-of) versus bitwise `&`, pointer arithmetic, and safe buffer interfaces. That is where we can move the display code into its own module without losing track of the memory it reads.
