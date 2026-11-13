# Lesson 14: Pointers, Object Lifetimes, and a GDB Memory Lab

> **Teaching approach:** Start with a short diagnostic. If the learner already understands `&`, `*`, basic pointer arithmetic, and pointer aliases, skip the introductory repetition and go directly to the practical lab. This chapter contains the full explanation for students who need it.
>
> **Repository status:** This is a learning chapter only. Do not assume that any example in this lesson has already replaced `src/main.c`. The learner should implement and test the example locally before committing source changes.

## Learning objectives

After this lesson you should understand:

- What a pointer stores, and how `&` and `*` relate an object to its address and value.
- Why `p + 1` moves by one **element**, not necessarily one byte.
- How two pointers may refer to the same object while remaining independent pointer variables.
- Why **all C function arguments are passed by value**, even if their values are pointers.
- The difference between `sizeof array` and `sizeof pointer`.
- What makes a pointer valid, null, one-past-the-end, or dangling.
- What the lifetime of a local object means and why returning its address is dangerous.
- How to view bytes, addresses, and pointer values directly in GDB.
- Why a safe byte-buffer interface typically needs **a pointer and a length**.

Prerequisites: [Lesson 08: Modules and Headers](08-modules-headers-and-separate-compilation.md), [Lesson 12: Arrays and sizeof](12-arrays-memory-layout-and-sizeof.md), and [Lesson 13: Multi-Byte Display](13-nested-loops-and-byte-display.md).

## 1. Diagnostic: predict before compiling

These problems work well as a short pre-lesson assessment. Explain *why*, not only the numerical result.

### Question A: a pointer changes the original object

```c
int value = 10;
int *p = &value;
*p = 25;
printf("%d\n", value);
```

### Question B: pointer arithmetic over an array

```c
int numbers[] = { 10, 20, 30, 40 };
int *p = numbers;

++p;
*p = 99;
printf("%d %d %d\n", numbers[0], numbers[1], numbers[2]);
```

### Question C: what is passed to a function?

```c
void change(int *p)
{
    *p = 50;
}
```

What does `change(&value)` change, and does C pass `p` by reference?

### Question D: independent pointer variables

```c
int a = 10;
int b = 20;
int *p = &a;
int *q = p;

q = &b;
*q = 77;
printf("%d %d\n", a, b);
```

### Question E: lifetime

```c
int *bad_address(void)
{
    int number = 42;
    return &number;
}
```

Can a caller safely dereference the returned pointer?

### Diagnostic answers

- A: prints `25`. The `*p` expression designates the same `int` object as `value`.
- B: prints `10 99 30`. Incrementing `p` moves it to the next `int` element.
- C: `change(&value)` stores 50 into the caller's object. But **the pointer value is passed by value**: the function receives its own local pointer variable.
- D: prints `10 77`. Reassigning `q` does not redirect `p`.
- E: **invalid to use after return**. The local automatic object's lifetime ends as the function returns; a pointer to it becomes indeterminate, and dereferencing it has undefined behavior.

**Path selection:** A learner who can explain all five should move directly to Sections 4–8. Others can review Sections 2 and 3 first.

## 2. The three questions to ask about every pointer

For an expression such as:

```c
int number = 10;
int *p = &number;
```

we should distinguish:

1. **The pointer object `p`:** a variable with type `int *`.
2. **The pointer's value:** the address that points to the `number` object.
3. **The pointed-to object:** the `int` that currently contains 10.

```text
Variable p                Variable number
+------------------+      +-----------+
| address of number |----->|    10     |
+------------------+      +-----------+
      int *                    int
```

The expression `p` evaluates to the pointer value. The expression `*p` accesses the pointed-to `int` object, assuming `p` is valid for that operation.

`&number` takes the address of `number`. This unary **address-of** operator is a different use of the `&` symbol from binary bitwise AND (`a & b`).

A pointer itself also has a storage location. We can take its address with `&p`, producing an `int **` (pointer to pointer), but that is a later topic.

## 3. Arrays and pointer arithmetic

Consider:

```c
int numbers[] = { 10, 20, 30 };
int *p = numbers;
```

In this initialization, the array expression `numbers` is converted to a pointer to its first element.

These expressions designate the same element:

```c
numbers[0]
*p
```

Similarly:

```c
numbers[1]
*(p + 1)
p[1]
```

**Pointer arithmetic uses element size.** If `sizeof(int) == 4` on a particular implementation, adding one to an `int *` moves to the next `int`, four C bytes later. It does not necessarily add one to the numeric address.

Pointer addition and subtraction are defined only within an appropriate array object (including the permitted one-past endpoint), not as a universal way to walk arbitrary process memory.

### The one-past pointer

For a three-element array, forming `numbers + 3` is permitted: it points **one past** the last element. **Dereferencing** that position is not permitted.

```c
int *end = numbers + 3; /* A valid one-past pointer value */
                          /* *end is NOT a valid array access */
```

Forming `numbers + 4` is not a valid pointer arithmetic operation for this array.

## 4. Crucial distinction: C passes pointers **by value**

This is the first practical difference to insist upon, especially for a programmer coming from C#.

```c
void rebind(int *p)
{
    p = NULL;
}
```

Calling `rebind(&number)` changes the **local** `p` inside `rebind`, not an external pointer variable in the caller. The parameter is a separate variable initialized with a copy of the supplied pointer value.

Compare:

```c
void overwrite(int *p)
{
    *p = 50;
}
```

That does not redirect the pointer; it modifies the original `int` object through the copied pointer value.

**A short rule:** Reassigning `p` changes the local pointer variable. Assigning `*p` changes the pointed-to object.

We will learn about `int **` when we need a function to update a caller's pointer variable.

## 5. Practical lab: aliases, movement, and memory bytes

This is the main hands-on experiment for MemView. Keep the normal helper module untouched.

### 5.1 Predict the result **before** building

Temporarily replace `src/main.c` with this example:

```c
#include <stddef.h>
#include <stdio.h>

/*
 * This function receives a COPY of the pointer value.
 * It can modify array elements but cannot redirect
 * the caller's pointer variable.
 */
static void move_local(unsigned char *current)
{
    ++current;
    *current = 0x77u;
}

int main(void)
{
    unsigned char data[] = {
        0x96u,
        0xA5u,
        0xFFu,
        0x00u
    };

    unsigned char *p = data;
    unsigned char *q = p;

    *q = 0x11u;
    ++p;

    move_local(p);

    printf("p index: %td\n", p - data);
    printf("q index: %td\n", q - data);

    for (size_t i = 0; i < sizeof data / sizeof data[0]; ++i)
    {
        printf("data[%zu] = 0x%02X\n",
               i,
               (unsigned int)data[i]);
    }

    printf("Array size: %zu\n", sizeof data);
    printf("Pointer size: %zu\n", sizeof p);

    return 0;
}
```

Questions to answer first:

1. Which element does `q` initially point to?
2. After `++p`, where does `p` point? Does `q` move too?
3. `move_local(p)` increments its **local** pointer. Which array element is modified?
4. Does `move_local` change the caller's `p`?
5. What four hex values will the loop print?
6. What will `p - data` and `q - data` produce?
7. Should `sizeof data` and `sizeof p` have the same value?

### 5.2 Build and compare

```bash
make
./build/memview
```

Expected on our 64-bit Debian lab machine (where an ordinary data pointer is eight bytes):

```text
p index: 1
q index: 0
data[0] = 0x11
data[1] = 0xA5
data[2] = 0x77
data[3] = 0x00
Array size: 4
Pointer size: 8
```

The two size values are **not necessarily the same on every implementation**, and pointer width is not guaranteed to be eight bytes by the C language. On our tested kind of 64-bit Linux environment this is the typical result.

What happened?

- `p` and `q` originally pointed to `data[0]`.
- `*q = 0x11u` changed `data[0]`.
- `++p` moved **only `p`** to `data[1]`.
- `move_local(p)` received a *copy* pointing to `data[1]`; its own `++current` moved to `data[2]` and wrote `0x77` there.
- On returning, the caller's `p` still pointed to `data[1]`.

### 5.3 What is `p - data`?

Subtracting two pointers into the same array yields the **difference in elements**, not bytes. The result has type `ptrdiff_t` from `<stddef.h>` and is printed with `%td`.

For a one-byte element array, the element difference also happens to match the byte offset. For an `int[]` array, the difference is still counted in **ints**, not in bytes.

Pointer subtraction requires pointers to appropriate elements of the same array object (or one past it), and the result must be representable in `ptrdiff_t`.

## 6. GDB lab: look at the actual bytes and addresses

Build for debugging using the existing Makefile (it already uses `-g -O0`):

```bash
make
gdb ./build/memview
```

Inside GDB:

```gdb
break main
run
list
```

Use `next` to execute the source lines until `p`, `q`, and `data` have been initialized. Then inspect:

```gdb
print p
print q
print &data[0]
print &data[1]
print sizeof data
print sizeof p
print p == q
x/4xb data
```

The `x/4xb data` command examines four units (`4`), in hexadecimal (`x`), as bytes (`b`), beginning at the array location (`data`).

Remember: do not inspect uninitialized variables before the corresponding initialization statements have executed. If you have stopped before the declarations run, use `next` first.

Now set a breakpoint on the helper:

```gdb
break move_local
continue
```

At the start of `move_local`, examine the local parameter and its target:

```gdb
print current
print *current
print &current
```

Use `next` to execute the increment. Then inspect `current` and `*current` again. You should see that the pointer now refers to a different array element.

When it returns:

```gdb
finish
print p
x/4xb data
```

Check that the **caller pointer** is unchanged even though one of its array's bytes was modified. You can also use `backtrace` while stopped inside the function to see the call stack.

Debugger output addresses are not fixed. The operating system may change them between runs. We care about **relationships**, not exact numeric addresses.

### A little more detail about address sizes

`sizeof p` gives the storage size of the pointer object `p`, not the number of elements of the original array.

These are different:

```c
sizeof data           /* total array size */
sizeof data[0]        /* one element */
sizeof p              /* pointer object size */
sizeof *p             /* pointed-to element size */
```

At the point of declaration in `main`, `data` is a **real four-element array**. In a function parameter such as `void inspect(unsigned char data[])`, the parameter is adjusted to a pointer type and `sizeof data` measures the pointer, not the caller's array.

## 7. Valid, null, one-past, and dangling pointers

There is more to a pointer than the numeric pattern that happens to represent an address.

- **Valid pointer to an object:** points to an object whose lifetime is active, with type and access rules satisfied.
- **Null pointer:** points to no object or function and must not be dereferenced. Use `NULL` or, where appropriate, a null pointer constant.
- **One-past pointer:** may be formed for a real array and used in certain comparisons or arithmetic; must **not** be dereferenced.
- **Dangling pointer:** points to an object whose lifetime has ended. Accessing the old object through it is not valid.

### Do not return the address of an automatic local variable

```c
int *bad_address(void)
{
    int number = 42;
    return &number; /* DANGER: object lifetime ends here */
}
```

The object `number` has automatic storage duration. Its lifetime ends when execution leaves its block. Under C's lifetime rules, a pointer to it becomes indeterminate after the lifetime ends. The caller cannot safely dereference the result.

It is possible that a debugger will still show `42` at the old address. That **does not prove that access is valid**.

A safe alternative is to let the caller own the result:

```c
void set_number(int *out)
{
    if (out != NULL)
    {
        *out = 42;
    }
}

int main(void)
{
    int number = 0;
    set_number(&number);
    return 0;
}
```

Here the caller's `number` remains alive during the call. The function does not return a pointer to a dead object.

We will examine other ownership patterns, including dynamic allocation, after understanding the core lifetime model.

## 8. A direction for safe MemView APIs

Our current demonstration obtains its count with:

```c
size_t count = sizeof data / sizeof data[0];
```

This is correct inside `main` where `data` is a real array.

A future module can instead accept a **pointer and length**:

```c
void display_bytes(const unsigned char data[], size_t count);
```

The `const` modifier promises that the function will not modify the byte elements through this parameter.

We should document and enforce a contract:

- `data` must point to at least `count` readable elements for a positive count.
- The caller is responsible for keeping those elements alive during the call.
- The implementation must never access `data[count]` as an element.
- The function should explicitly define its behavior when `count == 0`.
- A null pointer is **not** a substitute for a valid positive-length buffer.

Passing a pointer and count does not itself create memory safety. Correct bounds checks and a truthful caller-supplied length remain necessary.

Later we will create a separate formatting module, keep `helpers.c` focused on bit operations, and write tests for the new interface before implementing it.

## 9. Extra challenge (without running it)

What does this program change?

```c
void update(unsigned char *p)
{
    p[1] = 0x66u;
    ++p;
    *p = 0x55u;
}

int main(void)
{
    unsigned char values[] = { 0x10u, 0x20u, 0x30u };
    unsigned char *cursor = values;

    update(cursor);

    /* Predict the array and where cursor still points. */
    return 0;
}
```

Ask two separate questions:

1. Which array elements were actually modified?
2. Did the caller's `cursor` change?

The apparent repetition is intentional: both writes refer to the **same** element, but the second write wins. The caller's pointer value is unchanged.

## 10. Questions to take away

1. What is copied when calling a function with a pointer argument?
2. Why can `*p = 5` modify caller-owned data while `p = &other` cannot redirect the caller's pointer?
3. What does `p + 1` mean when `p` points to an `int`?
4. How does `sizeof array` differ from `sizeof pointer`?
5. What does `p - array` measure, and which format specifier prints its result?
6. What does the GDB command `x/4xb data` display?
7. Can forming a one-past pointer ever be legitimate? Can dereferencing it?
8. Why does seeing an old value in a debugger not guarantee a pointer is safe?
9. What does the `const` in `const unsigned char *data` restrict?
10. What extra information must a function receive to traverse a byte sequence safely?

## Summary

- **Pointers are values**, and C passes those values to functions **by value**.
- Dereferencing a valid pointer accesses the pointed-to object, but reassigning a local pointer only changes the local pointer variable.
- Pointer arithmetic is based on **elements**, and it is only defined within appropriate array bounds.
- Arrays and pointers are **not the same kind of object**, even though arrays often convert to a pointer to their first element.
- `sizeof` on a complete array is different from `sizeof` on a pointer.
- Validity depends on **lifetime and bounds**, not on an address merely looking plausible.
- GDB helps us connect source-level expressions to real addresses and bytes.

**Next:** After the learner has verified these predictions and inspected the results in GDB, we will design a small, test-first buffer-reading API for MemView (pointer + count + destination). Only then will we add new source modules and automated tests.
