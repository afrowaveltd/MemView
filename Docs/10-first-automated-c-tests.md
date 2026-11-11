# Lesson 10: Our First Automated Tests in C

> **Learning status:** We have successfully tried the first helper-function tests locally on Debian. This chapter records the working approach. The separate `tests/test_helpers.c` file and the `make test` target are still to be committed to the project repository; do not assume they are already present in a fresh checkout.

## What we will learn

- Why manually reading `printf` output does not scale.
- How `<assert.h>` helps check expected results automatically.
- How to test reusable functions independently of the application.
- Why a test executable can have its own `main()`.
- How to reuse an existing `.o` module from two separate executables.
- How to add an incremental `make test` target.
- Why tests must also demonstrate a controlled failure.
- What `assert()` cannot do, and when we will need a more structured test harness.

## 1. Our starting point

We already have a small module:

```text
MemView/
├── include/
│   └── helpers.h
├── src/
│   ├── main.c
│   └── helpers.c
├── Makefile
└── build/
```

Two functions exist in the public interface:

```c
int check_bit(unsigned char value, unsigned int position);
unsigned char set_bit(unsigned char value, unsigned int position);
```

The implementation is compiled into `build/helpers.o` and used by the main MemView application.

Until now, we often checked results by printing them:

```c
printf("Bit 5: %d\n", check_bit(value, 5u));
```

This is useful for learning, but not ideal as the number of functions and cases increases. We need the computer to compare **actual results** with **expected results**.

## 2. What does `assert()` do?

C provides an assertion macro in the standard header:

```c
#include <assert.h>
```

The expression inside `assert(expression)` should be true. For example:

```c
assert(2 + 2 == 4);
```

When the expression evaluates to nonzero, execution continues. When the expression evaluates to zero, an active assertion reports a failed condition and calls `abort()`, terminating the process abnormally. The diagnostic usually identifies the expression, file, and source line.

Experiment with:

```c
assert(2 + 2 == 5);
```

This is a **deliberately failing assertion**, not a compiler error.

**Important:** `assert` is a **macro**, not an ordinary function. If the macro `NDEBUG` is defined when `<assert.h>` is included, assertions are disabled. Therefore, we must not put necessary program logic or side effects inside assertions, and we should not build our intended test suite with `-DNDEBUG`.

An assertion is also not an error-handling strategy for ordinary untrusted program input. It is useful for checking assumptions and for a small learning-oriented test program.

## 3. Put test code in `tests/`

Create a separate directory:

```bash
mkdir -p tests
nano tests/test_helpers.c
```

Write:

```c
#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "helpers.h"

int main(void)
{
    unsigned char value = 0x81;

    /* Verify the bits that should already be set. */
    assert(check_bit(value, 0u) == 1);
    assert(check_bit(value, 7u) == 1);

    /* Verify a bit that should be clear. */
    assert(check_bit(value, 5u) == 0);

    /* Setting bit 5 changes 0x81 into 0xA1. */
    assert(set_bit(value, 5u) == 0xA1);

    /* Setting an already set bit changes nothing. */
    assert(set_bit(value, 0u) == value);

    /* The current API handles invalid positions safely. */
    assert(check_bit(value, CHAR_BIT) == 0);
    assert(set_bit(value, CHAR_BIT) == value);

    puts("All helper tests passed.");

    return 0;
}
```

These tests assume the familiar eight-bit byte of our Debian platform. The literal bit position `7` is valid when `CHAR_BIT >= 8`, which C guarantees. The value `0x81` also fits an unsigned char on all conforming C implementations.

### 3.1 Why test an invalid position?

Our two functions check the bit position before shifting. If `position >= CHAR_BIT`:

- `check_bit` currently returns `0`.
- `set_bit` currently returns its original value without modification.

This simple behavior protects against an invalid shift in our implementation. However, `check_bit` returning zero for an invalid position is indistinguishable from a valid, clear bit. A later API-design lesson should address explicit error reporting.

### 3.2 Tests describe a contract

Consider:

```c
assert(set_bit(value, 0u) == value);
```

This verifies an important *property*: setting an already-set bit must not accidentally change another bit.

Test cases should cover more than one example: typical inputs, boundary positions, repeated operations, and invalid positions.

## 4. Compile the test manually first

We already have `build/helpers.o` from our ordinary project build. If not, run:

```bash
make
```

Then compile the test executable separately:

```bash
gcc -std=c17 -O0 -g -Wall -Wextra -pedantic \
    -Iinclude \
    tests/test_helpers.c \
    build/helpers.o \
    -o build/test_helpers
```

Run it:

```bash
./build/test_helpers
```

Expected output:

```text
All helper tests passed.
```

We wrote this command explicitly so we can see every piece of the build rather than hide it in a Makefile immediately.

## 5. How can two files both define `main()`?

The application has `src/main.c` with its own `main()`.

The test program has `tests/test_helpers.c` with another `main()`.

This is legal because we compile **two separate executables**:

```text
src/main.c ------------------- build/main.o
                                   |
src/helpers.c -- build/helpers.o --+--> build/memview
                       |
                       +--> build/test_helpers
                              ^
                              |
                     tests/test_helpers.c
```

The object file `build/helpers.o` is reused by both executables. We do **not** link `main.o` and `test_helpers.c` together, which would create conflicting definitions of `main`.

The module's interface stays in `include/helpers.h`, so both the real application and the tests call the same functions.

## 6. Prove that the test can fail

A test that *always* reports success is not a useful test.

Temporarily change this assertion:

```c
assert(set_bit(value, 5u) == 0xA1);
```

to:

```c
assert(set_bit(value, 5u) == 0xFF);
```

Recompile the test executable and run it again. Because the expected value is now deliberately wrong, the assertion should fail and the program should terminate abnormally.

Immediately after the test command, run:

```bash
echo $?
```

A successful command normally reports an exit status of `0`; a failed assertion produces a nonzero status. The precise nonzero value and error message can vary by operating system and shell. On some systems, a failed assertion may also create a core dump.

**Now restore `0xA1` and rebuild.** The failure was only an experiment; we do not want to commit a deliberately broken test.

This experiment proves that the test checks real behavior rather than merely printing a success message.

## 7. Let GNU Make build and run tests

Once the manual compilation is understood, we can automate it.

In the root Makefile, add `test` to the phony list:

```makefile
.PHONY: all clean test
```

Add these rules:

```makefile
# --------------------------------------------------
# TESTS
# --------------------------------------------------

# Build the test program from its source and the shared helper object.
build/test_helpers: tests/test_helpers.c build/helpers.o include/helpers.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< build/helpers.o -o $@

# Run the executable; stop Make if a test fails.
test: build/test_helpers
	./build/test_helpers
```

Remember that the recipe lines begin with a **TAB**, not spaces.

In the `clean` rule, extend the existing removal command:

```makefile
clean:
	rm -f build/memview build/main.o build/helpers.o build/test_helpers
```

Now run:

```bash
make test
```

Make builds what is missing or out of date, then executes the test. A failed test returns a nonzero exit status, so the recipe fails and `make test` fails as well.

If we change `src/helpers.c`, Make notices that `build/helpers.o` needs recompiling. Since the test executable depends on that object file, Make then relinks the test program before running it.

### 7.1 Why is the first prerequisite the test source?

Our recipe uses `$<`:

```makefile
$(CC) $(CPPFLAGS) $(CFLAGS) $< build/helpers.o -o $@
```

The automatic variable `$<` expands to the **first prerequisite**, here `tests/test_helpers.c`.

`$@` expands to the target `build/test_helpers`.

The additional prerequisite `include/helpers.h` ensures the test executable is rebuilt if its interface changes. Our small Makefile lists header dependencies explicitly; later we can learn automatic dependency generation.

### 7.2 Why not make every build run all tests?

Our current default `all` target still builds the application. `make test` is an **explicit action** that also executes the tests. This keeps the purpose of each command clear:

```bash
make         # Build the application
make test    # Build and run tests
make clean   # Remove generated program and test outputs
```

When we introduce continuous integration, it can run both the ordinary build and the tests automatically.

## 8. What have we learned about automated verification?

A good first test has three parts:

1. **Arrange:** Prepare an input (for example `value = 0x81`).
2. **Act:** Call the code under test (for example `set_bit(value, 5u)`).
3. **Assert:** Compare the actual result with the expected result.

The names **Arrange–Act–Assert** are often used in test design, but the mechanism is simply ordinary C control flow plus a checking macro.

For now our tests are simple. Later we will want clearer per-test names, counts of passed/failed cases, more informative diagnostics, and a framework that can report multiple failures in one run.

## 9. Next experiments

When the helper module gains more functions, we will add cases for:

- `clear_bit`: clearing a set bit and clearing an already-clear bit.
- `toggle_bit`: flipping a zero to one, flipping a one to zero, and toggling twice to restore the original value.
- Bit positions at the edges of a byte.
- Invalid bit positions.
- Consistency properties between the helper operations.

We will also learn the difference between a basic example test, a boundary test, a regression test, and a property-style test.

**Principle:** Keep the program modular, keep tests independent of `main.c`, and confirm that an intentionally incorrect expectation actually causes a failure.
