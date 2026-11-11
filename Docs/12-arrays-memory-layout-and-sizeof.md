# Lesson 12: Arrays, Contiguous Memory, and sizeof

> **Starting point:** We already know variables, integer types, bit operations, loops, and our reusable bit helpers. Now we will connect several values to their **physical arrangement as C objects**. This is the foundation for displaying byte sequences in MemView.

## Learning objectives

At the end of this lesson, students should be able to:

- Declare an array and explain what its elements are.
- Understand that C array elements occupy consecutive regions of storage.
- Distinguish an array's **index** from a bit's **position**.
- Use `sizeof` to measure the array and one element.
- Calculate an array's element count when the complete array type is known.
- Use `size_t` and the `%zu` format specifier correctly.
- Explain why the bounds of a C array must be respected.
- Recognize the difference between an array object and a pointer, particularly at function boundaries.
- Inspect the byte sequence without modifying the underlying values.
- Connect arrays to the eventual MemView hex and binary display.

**Prerequisites:** [Lesson 06: C Integer Types, Bytes, and Signedness](06-c-integer-types.md), [Lesson 08: Functions and Separate Compilation](08-modules-headers-and-separate-compilation.md), [Lesson 11: Loops and Binary Display](11-loops-and-binary-output.md).

## 1. Why does a memory viewer need arrays?

Until now, most experiments used **one** number:

```c
unsigned char value = 0x96u;
```

But an actual memory display needs to show a **sequence of bytes**. An array is a natural place to begin.

```c
unsigned char bytes[] = {
    0x96u,
    0xA5u,
    0xFFu,
    0x00u
};
```

We have declared a single array object consisting of **four elements**, each of type `unsigned char`.

The empty brackets `[]` here mean that the compiler deduces the array length from the initializer list. We could also declare it explicitly:

```c
unsigned char bytes[4] = {
    0x96u,
    0xA5u,
    0xFFu,
    0x00u
};
```

The same four objects and initial values result.

Notice that we did not use an allocation API or write `new`. Local arrays of fixed known size can be declared directly in C. Where an object is stored depends on its storage duration and the implementation; it is not correct to claim that every C array must be on the stack or heap.

## 2. What does contiguous storage mean?

C requires array elements to be allocated **contiguously**. Conceptually:

```text
Index:             0       1       2       3
                 +-------+-------+-------+-------+
Value (hex):     |  96   |  A5   |  FF   |  00   |
                 +-------+-------+-------+-------+
Offset (bytes):   +0      +1      +2      +3
```

Each `unsigned char` occupies **exactly one C byte**. On our Debian x86-64 system, a byte has eight bits (`CHAR_BIT == 8`).

This is an important distinction:

- `bytes[0]`, `bytes[1]`, etc. identify **array elements**.
- Bit positions within each element are numbered independently from 0 through `CHAR_BIT - 1`.
- The relative byte offsets shown above are independent of whether we choose to *print* each byte's bits MSB-first or LSB-first.

The values can be interpreted in binary:

```text
bytes[0] = 0x96 = 10010110
bytes[1] = 0xA5 = 10100101
bytes[2] = 0xFF = 11111111
bytes[3] = 0x00 = 00000000
```

See [Lesson 11](11-loops-and-binary-output.md) for why bit positions and display order are separate concepts.

### Does "contiguous" mean an array never contains padding?

The next array element starts exactly `sizeof(element type)` C bytes after the preceding one. There are no *extra gaps between array elements*.

Some types (such as structures) may contain **padding within their own object representation**, and `sizeof` includes that padding. We will study that when we reach `struct`.

For `unsigned char`, however, `sizeof(unsigned char)` is always 1, so adjacent elements of our byte array occupy adjacent C bytes.

## 3. What does `sizeof` actually measure?

The `sizeof` operator measures the size of a type or object in **C bytes**.

```c
sizeof bytes
sizeof bytes[0]
sizeof(unsigned char)
```

For our four-element array, the results are:

```text
sizeof bytes          = 4
sizeof bytes[0]       = 1
sizeof(unsigned char) = 1
```

A subtle rule: in C, `sizeof(char)` is **always 1**, but a C byte is not guaranteed to contain exactly eight bits on every C implementation. To find the number of bits in a byte, use `CHAR_BIT` from `<limits.h>`.

For ordinary fixed-size objects such as our array, `sizeof` gives a size known from the type. We will leave variable-length arrays and some special `sizeof` rules for later.

### `sizeof` is not an ordinary function

These are both common forms:

```c
sizeof bytes          /* Operand is an expression */
sizeof(unsigned char) /* Operand is a type name, so parentheses are required */
```

The parentheses around `bytes` in `sizeof(bytes)` are optional; those around a type name are not.

## 4. How do we determine the element count?

The array occupies four bytes, and each element occupies one byte:

```c
size_t count = sizeof bytes / sizeof bytes[0];
```

The formula is general for a **real array whose complete type is visible here**, not only for bytes:

```text
total size in bytes / one element's size in bytes
    = number of elements
```

Examples:

- For four `unsigned char` values: `4 / 1 = 4`.
- If some other element type has `sizeof` equal to 4 on the target platform, an array of six such objects occupies `24` bytes, and `24 / 4 = 6`.

We intentionally use `sizeof bytes[0]` instead of hardcoding `1`. If we later change the type of the array elements, the formula continues to reflect their actual size.

### What is `size_t`?

The result type of `sizeof` is `size_t`, an **unsigned integer type** capable of representing object sizes supported by the implementation.

We can obtain its declaration from `<stddef.h>`, and the formatted output specifier for a `size_t` value is `%zu`:

```c
#include <stddef.h>
#include <stdio.h>

size_t count = sizeof bytes / sizeof bytes[0];
printf("Count: %zu\n", count);
```

Use `%zu`, not `%d`, when printing an unconverted `size_t` value. The correct format must match the argument's type.

## 5. Array indexing and bounds

Arrays in C are indexed starting at **zero**:

```c
bytes[0] /* 0x96 */
bytes[1] /* 0xA5 */
bytes[2] /* 0xFF */
bytes[3] /* 0x00 */
```

These expressions select the actual array elements. They are **not** copies of the entire array.

For this particular array, the valid indices satisfy:

```text
0 <= index < 4
```

The expression:

```c
bytes[4] /* OUT OF BOUNDS -- do not evaluate */
```

attempts to read beyond the array. That has **undefined behavior** in C.

Unlike ordinary managed C# array access, C does not promise a runtime bounds exception or any automatic check here. The program might crash, seem to work, display unrelated data, or behave unpredictably. None of those outcomes can be relied upon.

A pointer referring **one past** an array's end has some legitimate uses in C, but **dereferencing** that location is not valid. We will return to this distinction when we learn pointers.

### Correct loop boundary

```c
for (size_t index = 0; index < count; ++index)
{
    /* Access bytes[index] safely for this array. */
}
```

This loop uses `index < count`, not `index <= count`. The latter would execute once at `index == count` and go beyond the last element.

## 6. Our first array experiment in MemView

This is the experiment to write manually in `src/main.c`. We will **not** change the actual project source in the repository as part of preparing this lesson.

```c
#include <stddef.h>
#include <stdio.h>

int main(void)
{
    unsigned char bytes[] = {
        0x96u,
        0xA5u,
        0xFFu,
        0x00u
    };

    size_t count = sizeof bytes / sizeof bytes[0];

    printf("Array size:   %zu bytes\n", sizeof bytes);
    printf("Element size: %zu byte\n", sizeof bytes[0]);
    printf("Element count: %zu\n", count);

    for (size_t index = 0; index < count; ++index)
    {
        printf("bytes[%zu] = 0x%02X\n",
               index,
               (unsigned int)bytes[index]);
    }

    return 0;
}
```

Build and run on Debian:

```bash
make
./build/memview
```

Expected result on our eight-bit-byte system:

```text
Array size:   4 bytes
Element size: 1 byte
Element count: 4
bytes[0] = 0x96
bytes[1] = 0xA5
bytes[2] = 0xFF
bytes[3] = 0x00
```

The `%02X` format prints the value in uppercase hexadecimal with a **minimum width** of two characters, padding with leading zeros when needed. The conversion to `unsigned int` makes the argument type explicitly match the `%X` conversion specifier.

At this point we can display a sequence of bytes rather than a single value.

## 7. Inspect memory addresses: an optional second experiment

Our diagram showed **relative offsets**. Let us verify them with actual addresses, without assuming the process starts at any particular address.

Add this loop after the ordinary value-printing loop:

```c
for (size_t index = 0; index < count; ++index)
{
    printf("&bytes[%zu] = %p\n",
           index,
           (void *)&bytes[index]);
}
```

This introduces two new ideas:

- `&bytes[index]` forms a **pointer** to that element.
- The `%p` conversion prints a pointer's representation; casting to `void *` supplies the expected argument type.

On an ordinary system where each `unsigned char` occupies one byte, you should observe successive addresses advancing by one byte. The precise textual appearance of pointers is implementation-defined and the actual addresses can vary between runs due to operating-system memory layout and address randomization.

For example, this is **illustrative only**, not a promised address:

```text
&bytes[0] = 0x7fff12345000
&bytes[1] = 0x7fff12345001
&bytes[2] = 0x7fff12345002
&bytes[3] = 0x7fff12345003
```

Do not try to store or compare those addresses as fixed numbers. For this lesson, the relevant observation is **the relationship between adjacent element addresses**.

We are not teaching arbitrary memory access yet; our pointers refer to the elements of a valid existing array.

## 8. Array versus pointer: the important C-specific trap

This is where C differs significantly from C#.

In `main()`:

```c
unsigned char bytes[] = { 0x96u, 0xA5u, 0xFFu, 0x00u };

size_t count = sizeof bytes / sizeof bytes[0]; /* 4 */
```

Here `bytes` is an **array object** with a complete known type and size.

But consider a function parameter:

```c
void inspect(unsigned char data[], size_t count)
{
    /* Use count to decide how many elements to inspect. */
}
```

Within the function parameter list, `unsigned char data[]` is **adjusted to a pointer parameter**. In this function, `data` does not itself contain or automatically reveal the length of the caller's array.

The expression:

```c
sizeof data / sizeof data[0]
```

**must not be used to recover the number of elements there**: it measures the size of the pointer parameter, not the original array object.

This is why a common C interface takes **both** the location of the first element and an explicit element count:

```c
void inspect(const unsigned char data[], size_t count);
```

The `const` qualifier prevents this function from modifying the elements through the `data` parameter. It does not make invalid indices safe.

There are special rules about when array expressions convert to pointers and when they do not, including `sizeof`. We will explore these step by step when we study pointers.

**Key distinction:** An array object is not identical to a pointer, even though array expressions frequently convert to pointers in many contexts.

## 9. Initialization: another difference from managed C#

A C array can be initialized in several ways:

```c
unsigned char first[] = { 1u, 2u, 3u };

unsigned char second[4] = { 0u }; /* All four elements become zero. */

unsigned char third[4] = { 1u, 2u };
/* The omitted remaining elements are initialized to zero. */
```

An ordinary automatic local array declared **without an initializer**:

```c
unsigned char unknown[4];
```

does **not** automatically give you four known zero values. Its elements have indeterminate initial values, and you should initialize them before using them.

This is not the same expectation as a newly allocated managed C# array, where the runtime provides default-initialized elements.

For a memory viewer, be deliberate about which buffers are initialized, which ones contain bytes read from somewhere, and which elements are valid for inspection.

## 10. Preparing for a hex dump

We can now combine two ideas:

1. An array gives us a **sequence of byte values**.
2. `check_bit()` gives us the state of a particular bit **within a selected byte**.

A small future preview looks like this:

```text
Index  Hex  Binary
-----  ---  --------
0      96   10010110
1      A5   10100101
2      FF   11111111
3      00   00000000
```

To generate this table, we can use an **outer loop** over the array and an **inner loop** over bit positions.

However, before expanding the example, first make sure that the address, index, byte size, and total element count are understood. A real hex dump will also need formatting, line breaks, offsets, and a clear policy for which bytes may safely be read.

### One more architectural point

Our `helpers.c` module should remain responsible for **bit calculations**. We will later put output formatting in its own place instead of turning the helpers module into a large mixture of bit logic and terminal display code.

## 11. Questions to test understanding

1. How many objects are created by `unsigned char data[] = { 4u, 5u, 6u };`?
2. What is `sizeof data` for that array, measured in C bytes?
3. Why are the valid indices `0` through `2` rather than `1` through `3`?
4. Why is `sizeof data / sizeof data[0]` correct in the local array's scope but not necessarily inside a function taking `unsigned char data[]` as a parameter?
5. Does using `char` guarantee that a C byte has exactly eight bits on *all* platforms?
6. Does `bytes[4]` necessarily produce an exception?
7. Why do we use `size_t` and `%zu` when printing object sizes?
8. What is the difference between the memory **offset** of `bytes[2]` and the binary **bit positions** within `bytes[2]`?
9. What happens if we declare a local array without initializing it?
10. How would we combine an outer array loop with the inner bit loop from the previous lesson?

## Summary

- C arrays contain a fixed number of contiguously arranged elements.
- `sizeof` measures C bytes, not automatically eight-bit octets.
- An array's known size can determine its element count **at the point where the array type is still visible**.
- Indexing starts at zero; accessing an element beyond the bounds has undefined behavior.
- A function parameter written with `[]` behaves as a pointer parameter and does not automatically carry the original array's element count.
- We can now explain and display several consecutive bytes—an essential foundation for MemView.

**Next:** Nested loops and the first multi-byte hex-and-binary view, followed by an introduction to pointers, array-to-pointer conversion, and safe buffer interfaces.
