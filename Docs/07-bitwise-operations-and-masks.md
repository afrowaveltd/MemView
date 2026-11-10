# Lesson 07: Bitwise Operations, Shifts, and Masks

## Learning objectives

By the end of this lesson, you should understand:

- The difference between binary representations and numerical values.
- What `&`, `|`, `^`, and ``` do when applied to integer operands.
- How `<<` and `>>` move bits and what their limitations are.
- Why bit positions are numbered starting at zero from the right.
- How to create a mask with `1u << position`.
- How to test a single bit without changing any other bits.
- How C's integer promotions affect small types such as `unsigned char`.

The experiments assume the project-root `Makefile` from [Lesson 05](05-gnu-make.md). We temporarily replace `src/main.c` with each sample, run `make`, and inspect the result. You can restore your preferred source file afterward.

## 1. Why MemView needs bitwise operations

A byte is a numerical value. We can display that value as decimal, hexadecimal, or binary; the contents do not change just because their presentation changes.

For example, on our eight-bit-byte computer:

```text
Decimal:      165
Hexadecimal:  0xA5
Binary:       10100101
```

A memory viewer must sometimes read one individual bit, not only the entire byte. This is exactly what bitwise operations help us do.

**Terminology note:** In C, the logical operators `&&`, `||`, and `!` work with truth values and comparisons. The bitwise operators `&`, `|`, `^`, and ``` work on the bits of integer operands. Although C# also allows certain operators on `bool` values, we are studying the *integer bitwise* meaning here.

## 2. Bitwise AND, OR, XOR, and NOT

Start with two unsigned eight-bit values:

```c
unsigned char a = 0xF0; /* 11110000 */
unsigned char b = 0x0F; /* 00001111 */
```

### 2.1 AND: `&`

A result bit is 1 only when **both** input bits are 1:

```text
11110000
00001111
--------
00000000
```

So `a & b` produces zero.

### 2.2 OR: `|`

A result bit is 1 when **either** input bit is 1:

```text
11110000
00001111
--------
11111111
```

So `a | b` produces `0xFF`.

### 2.3 XOR: `^`

A result bit is 1 when the two input bits are **different**:

```text
11110000
00001111
--------
11111111
```

So `a ^ b` also produces `0xFF` for these particular inputs.

### 2.4 NOT: ``` 

NOT flips every bit. But there is an important C-specific detail: in expressions like ` `a`, an `unsigned char` operand is **integer-promoted** (usually to `int` on our system) before applying the operator.

For that reason, when we want to show only the low eight bits, we explicitly convert the result back to `unsigned char`:

```c
(unsigned char)(`a)
```

For our eight-bit sample, the resulting byte is `0x0F`.

### 2.5 Working experiment

```c
#include <stdio.h>

int main(void)
{
    unsigned char a = 0xF0; /* 11110000 */
    unsigned char b = 0x0F; /* 00001111 */

    printf("a     = 0x%02X\n", (unsigned int)a);
    printf("b     = 0x%02X\n", (unsigned int)b);

    printf("a & b = 0x%02X\n", (unsigned int)(a & b));
    printf("a | b = 0x%02X\n", (unsigned int)(a | b));
    printf("a ^ b = 0x%02X\n", (unsigned int)(a ^ b));
    printf("`a    = 0x%02X\n", (unsigned int)(unsigned char)(`a));

    return 0;
}
```

Build and run:

```bash
make
./build/memview
```

Try changing `a` to `0xAA` and `b` to `0x55`, and predict the four results before running the program.

## 3. Moving bits: left shift and right shift

The operators

```c
value << 1
value >> 1
```

shift the bit representation by one position to the left or to the right.

For example, start with the positive value `44` (hex `0x2C`):

```text
Original:      00101100   = 44
Left shift 1:  01011000   = 88
Right shift 1: 00010110   = 22
```

For **unsigned values** within a suitable range, a left shift by one has the effect of multiplying by two modulo the width of the unsigned result type. A right shift of an unsigned value by one has the effect of integer division by two.

These are useful intuitions, but do not assume shifts always behave exactly like ordinary multiplication or division for signed integers, especially negative ones.

### 3.1 A subtle trap: the promoted type

Consider:

```c
unsigned char value = 0xA5;  /* 165 */
unsigned int wider = value << 1;
```

On our Debian platform, `value` is promoted to `int` first. Consequently `value << 1` produces the wider value `330` (`0x14A`), which is representable in `int`. It does **not** become `0x4A` simply because the original object was one byte.

If we **convert** the result back to an eight-bit `unsigned char`, we keep only the low eight bits, yielding `0x4A`:

```c
unsigned char byte_result = (unsigned char)(value << 1);
```

This is another example of why the type of the *object* and the type of an *intermediate expression* may differ.

## 4. Bit positions

We number positions starting at **zero on the right**, at the least significant bit:

```text
Position: 7 6 5 4 3 2 1 0
Value:    1 0 1 0 0 1 0 1
```

This is `0xA5`. Bits 7, 5, 2, and 0 are set.

Why count from the right? Because each position represents a power of two:

| Position | Value when set |
| --- | ---: |
| 0 | 1 |
| 1 | 2 |
| 2 | 4 |
| 3 | 8 |
| 4 | 16 |
| 5 | 32 |
| 6 | 64 |
| 7 | 128 |

Thus, `0xA5 = 128 + 32 + 4 + 1 = 165`.

## 5. A mask selects an individual bit

We want to check whether bit **5** is set in `0xA5`.

Start with an unsigned integer whose only set bit is bit zero:

```c
1u
```

The suffix `u` makes this constant an `unsigned int`. Move the bit to position five:

```c
1u << 5
```

On our system, the low eight bits of this value are:

```text
00100000 = 0x20
```

This is a **bit mask**. Combine it with the original value using AND:

```text
Value:       10100101
Mask:        00100000
             --------
AND result:  00100000
```

The result is nonzero, so bit 5 is set.

Now try position 3:

```text
Value:       10100101
Mask:        00001000
             --------
AND result:  00000000
```

The result is zero, so bit 3 is clear.

The mask lets us select one particular bit **without changing the original value**.

## 6. Our first bit-reading program

Open `src/main.c` and enter:

```c
#include <stdio.h>

int main(void)
{
    unsigned char value = 0xA5; /* 10100101 */

    unsigned int mask5 = 1u << 5;
    unsigned int mask3 = 1u << 3;

    printf("Value: 0x%02X\n", (unsigned int)value);

    printf("Mask 5: 0x%02X\n", mask5);
    printf("Mask 3: 0x%02X\n", mask3);

    printf("Bit 5: %u\n", ((unsigned int)value & mask5) >> 5);
    printf("Bit 3: %u\n", ((unsigned int)value & mask3) >> 3);

    return 0;
}
```

Then run:

```bash
make
./build/memview
```

Expected output:

```text
Value: 0xA5
Mask 5: 0x20
Mask 3: 0x08
Bit 5: 1
Bit 3: 0
```

How does it work?

1. `1u << position` creates a mask with one selected bit.
2. `value & mask` removes every bit other than the selected bit.
3. `>> position` moves that selected bit down to bit zero, giving `0` or `1`.

This is our first practical step toward displaying individual bits in MemView.

### 6.1 Safety limits for shifts

The shift amount must be **nonnegative and less than the width of the promoted left operand**. For example, if `unsigned int` is 32 bits wide, `1u << 32` is not valid C behavior.

Our examples use positions 3 and 5, which are valid on every conforming implementation because `unsigned int` is at least 16 bits wide.

For a general-purpose bit viewer, we must validate the selected position against the actual width of the object we are examining.

## 7. Paper exercise: read three bits

Take the value:

```c
unsigned char value = 0x81;
```

Convert it to binary:

```text
0x81 = 10000001
```

Without running the program, determine whether these bits are set:

- Bit 7
- Bit 5
- Bit 0

Check using the mask formula:

```c
(value & (1u << position)) != 0u
```

Expected results: bit 7 is **1**, bit 5 is **0**, and bit 0 is **1**.

Notice that `!= 0u` converts the masked result to a truth test. This is often convenient when we want to use the result in an `if` statement.

## 8. What comes next?

Reading a bit is only the beginning. We will soon use masks to:

- **Set** a chosen bit while leaving the others unchanged.
- **Clear** a chosen bit.
- **Toggle** a chosen bit.
- Read bit groups and eventually show all eight bits of a byte.

We will also learn how to avoid assumptions about signed representations and how to present raw bytes accurately.

The guiding principle remains: **understand the bytes, then write the viewer**.
