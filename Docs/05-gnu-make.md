# Lesson 05: GNU Make — From Explicit Commands to Reusable Rules

## Learning objectives

By the end of this lesson, you should be able to:

- Explain the roles of a **target**, its **prerequisites**, and its **recipe**.
- Build a small C program from two source files.
- Understand why a dependency is mentioned in a rule and passed to GCC.
- Use comments to document a Makefile.
- Read and write `CC` and `CFLAGS` variables.
- Recognize the automatic variables `$@`, `$<`, and `$^`.
- Replace two nearly identical recipes with a **pattern rule**.
- Use a **dry run** to inspect planned commands.
- Understand how Make decides what needs rebuilding.

We will deliberately show the **evolution** of one Makefile. The shortest solution is not necessarily the easiest one to understand.

## 1. The problem: Repeating compilation commands

In [Lesson 04](04-first-c-program.md), we built a program containing two source files:

- `build/linux-native/link_demo_main.c` declares and calls `get_value()`.
- `build/linux-native/link_demo_value.c` defines `get_value()`.

For reference, the source files are:

```c
/* link_demo_main.c */
int get_value(void);

int main(void)
{
    return get_value();
}
```

```c
/* link_demo_value.c */
int get_value(void)
{
    return 7;
}
```

We can build the program manually from the repository root:

```bash
mkdir -p build/linux-native

gcc -std=c17 -O0 -Wall -Wextra -pedantic -c build/linux-native/link_demo_main.c -o build/linux-native/link_demo_main.o
gcc -std=c17 -O0 -Wall -Wextra -pedantic -c build/linux-native/link_demo_value.c -o build/linux-native/link_demo_value.o
gcc build/linux-native/link_demo_main.o build/linux-native/link_demo_value.o -o build/linux-native/link_demo
```

That is manageable with two modules, but cumbersome with twenty or fifty. We also want to **avoid recompiling files that have not changed**.

GNU Make reads rules that describe the relationships between input and output files.

**Working-directory convention:** Throughout this lesson, we run `make` from the **repository root** with `-f build/linux-native/Makefile`. All paths shown in rules are relative to that working directory. Placing the Makefile in a subdirectory does not automatically change Make's working directory.

Our experimental `build/` directory is ignored by Git. This lesson documents the experiments without making those temporary source files part of the committed MemView project.

## 2. First version: A fully explicit linking rule

Create `build/linux-native/Makefile` and begin with just one rule:

```makefile
build/linux-native/link_demo: build/linux-native/link_demo_main.o build/linux-native/link_demo_value.o
	gcc build/linux-native/link_demo_main.o build/linux-native/link_demo_value.o -o build/linux-native/link_demo
```

A Make rule follows this basic shape:

```makefile
target: prerequisite1 prerequisite2
	command to build the target
```

**The first line is for Make.** It says that `link_demo` depends on two object files. Make can compare modification times and decide whether the executable is out of date.

**The second line is the recipe.** It tells Make *how* to build the executable, in this case by calling GCC to perform linking.

This explains the apparent duplication: Make does **not** automatically pass the first line's prerequisites to GCC as arguments. They serve different purposes.

> **Very important:** By default, recipe command lines begin with a literal **TAB character**, not spaces. The other lines in this example do not require indentation.

Test the rule:

```bash
make -f build/linux-native/Makefile
```

If the two object files already exist and the executable is newer, Make may report:

```text
make: 'build/linux-native/link_demo' is up to date.
```

At this stage, Make still does **not** know how to create a missing `.o` file. The rule only handles linking.

## 3. Second version: Documenting and completing the rules

Comments begin with `#` in Makefile syntax. We use them to record the **reason** for a rule and what its options mean.

Now replace the Makefile with the following complete explicit version:

```makefile
# MemView - learning exercise for incremental builds
#
# Run from the repository root:
#   make -f build/linux-native/Makefile

# --------------------------------------------------
# 1. LINKING
# --------------------------------------------------

# The executable depends on BOTH object files.
# If either object is newer, we must link again.
build/linux-native/link_demo: build/linux-native/link_demo_main.o build/linux-native/link_demo_value.o
	gcc build/linux-native/link_demo_main.o build/linux-native/link_demo_value.o -o build/linux-native/link_demo

# --------------------------------------------------
# 2. COMPILING MAIN
# --------------------------------------------------

# -c creates an object file and stops before linking.
build/linux-native/link_demo_main.o: build/linux-native/link_demo_main.c
	gcc -std=c17 -O0 -Wall -Wextra -pedantic -c build/linux-native/link_demo_main.c -o build/linux-native/link_demo_main.o

# --------------------------------------------------
# 3. COMPILING THE IMPLEMENTATION
# --------------------------------------------------

# Changing this C source should rebuild its object,
# but should not force an unrelated source to recompile.
build/linux-native/link_demo_value.o: build/linux-native/link_demo_value.c
	gcc -std=c17 -O0 -Wall -Wextra -pedantic -c build/linux-native/link_demo_value.c -o build/linux-native/link_demo_value.o
```

Notice that the first rule appears before the two compilation rules. That is fine. Make builds a **dependency graph** and can discover how to update prerequisites before it runs the linking recipe.

By default, the first ordinary target in this file becomes the default goal. Here it is `build/linux-native/link_demo`.

### 3.1 Verify incremental rebuilding

First run Make:

```bash
make -f build/linux-native/Makefile
```

Run it a second time without changing anything. It should find the result up to date.

Next, update the modification time of **one source file**, without changing its contents:

```bash
touch -m build/linux-native/link_demo_value.c
make -f build/linux-native/Makefile
```

Normally, two recipes should run:

1. Recompile `link_demo_value.c` into `link_demo_value.o`.
2. Relink the executable because that object is now newer.

`link_demo_main.c` does not need recompilation.

### 3.2 What does `touch -m` do?

The `touch` utility can update file timestamps. The `-m` option selects the **modification time**. This is an easy way to simulate a changed input without modifying its source text.

**Make normally uses timestamps**, not a content hash, for this basic kind of out-of-date detection. Accurate dependency lists and sensible timestamps therefore matter.

## 4. Third version: Ordinary Make variables

The fully explicit version repeats `gcc` and the same compiler options. We can move them into variables:

```makefile
# The C compiler. CC is a conventional Make variable name.
CC = gcc

# Compiler options shared by our C source files.
# -std=c17  : select the C17 language standard
# -O0       : disable normal optimizations for simpler debugging
# -Wall     : enable a useful set of warnings
# -Wextra   : enable additional warnings
# -pedantic : diagnose certain extensions/nonconforming constructs
CFLAGS = -std=c17 -O0 -Wall -Wextra -pedantic
```

To refer to these variables, use `$(CC)` and `$(CFLAGS)`.

Our linking rule becomes:

```makefile
# Link the object files into an executable.
build/linux-native/link_demo: build/linux-native/link_demo_main.o build/linux-native/link_demo_value.o
	$(CC) build/linux-native/link_demo_main.o build/linux-native/link_demo_value.o -o build/linux-native/link_demo
```

And the two compile rules become:

```makefile
# Compile the main source file.
build/linux-native/link_demo_main.o: build/linux-native/link_demo_main.c
	$(CC) $(CFLAGS) -c build/linux-native/link_demo_main.c -o build/linux-native/link_demo_main.o

# Compile the function implementation.
build/linux-native/link_demo_value.o: build/linux-native/link_demo_value.c
	$(CC) $(CFLAGS) -c build/linux-native/link_demo_value.c -o build/linux-native/link_demo_value.o
```

The commands executed by the shell are still ordinary `gcc` commands; Make expands the variables before executing the recipes.

Changing the compiler or shared flags now requires editing one location instead of every rule.

> `CC` and `CFLAGS` are conventions recognized by many Makefiles and built-in rules, not keywords of the C language. A simple assignment using `=` is sufficient for our current exercise.

## 5. Fourth version: Automatic variables

We still repeat long filenames. GNU Make can substitute values based on the **current rule**:

| Symbol | Meaning | Example in our link rule |
| --- | --- | --- |
| `$@` | Current target | `build/linux-native/link_demo` |
| `$<` | First prerequisite | `build/linux-native/link_demo_main.o` |
| `$^` | All prerequisites, with duplicate entries removed | Both `.o` files |

These are called **automatic variables**. They have meaning in the context of a rule's recipe; they are not general shell variables.

We can now write:

```makefile
# C compiler and options.
CC = gcc
CFLAGS = -std=c17 -O0 -Wall -Wextra -pedantic

# $^ = all object file prerequisites
# $@ = current target executable
build/linux-native/link_demo: build/linux-native/link_demo_main.o build/linux-native/link_demo_value.o
	$(CC) $^ -o $@

# $< = C source; $@ = object to create
build/linux-native/link_demo_main.o: build/linux-native/link_demo_main.c
	$(CC) $(CFLAGS) -c $< -o $@

# Identical command, but Make supplies different filenames.
build/linux-native/link_demo_value.o: build/linux-native/link_demo_value.c
	$(CC) $(CFLAGS) -c $< -o $@
```

The linking recipe:

```makefile
	$(CC) $^ -o $@
```

expands to:

```bash
gcc build/linux-native/link_demo_main.o build/linux-native/link_demo_value.o -o build/linux-native/link_demo
```

And the second compiling recipe:

```makefile
	$(CC) $(CFLAGS) -c $< -o $@
```

expands to:

```bash
gcc -std=c17 -O0 -Wall -Wextra -pedantic -c build/linux-native/link_demo_value.c -o build/linux-native/link_demo_value.o
```

We have **not changed the dependency graph**. We have only made the recipes easier to maintain.

### 5.1 View expansions without compiling

Use a dry run:

```bash
make -n -B -f build/linux-native/Makefile
```

- `-n` (or `--just-print`) normally prints the recipes without running them.
- `-B` (or `--always-make`) forces Make to consider the targets out of date.

With our simple recipes, this prints both compilation commands and the linking command, with the automatic variables expanded.

This is a safe and convenient way to **inspect what a Makefile intends to do**. Note that Make has special cases for recursive Make invocations; `-n` is not a universal security sandbox for arbitrary Makefiles.

## 6. Fifth version: A pattern rule

Now both compiling recipes contain exactly the same instruction:

```makefile
	$(CC) $(CFLAGS) -c $< -o $@
```

Only their source and object filenames differ.

Instead of writing a separate rule for every source file, we can describe the relationship once with the `%` wildcard:

```makefile
# An object under build/linux-native depends on
# a C source file with the same basename.
build/linux-native/%.o: build/linux-native/%.c
	$(CC) $(CFLAGS) -c $< -o $@
```

Examples of matching filenames:

```text
build/linux-native/link_demo_main.c  -> build/linux-native/link_demo_main.o
build/linux-native/link_demo_value.c -> build/linux-native/link_demo_value.o
```

The `%` represents the part of the filename called the **stem**. In this case it stands for `link_demo_main` or `link_demo_value`.

### 6.1 Our first compact, commented Makefile

Replace the file with this version:

```makefile
# MemView - GNU Make learning exercise
#
# Run from the repository root:
#   make -f build/linux-native/Makefile

# Toolchain
CC = gcc

# Compilation options
CFLAGS = -std=c17 -O0 -Wall -Wextra -pedantic

# The final program depends on both object files.
# $^ expands to the prerequisites; $@ to the target.
build/linux-native/link_demo: build/linux-native/link_demo_main.o build/linux-native/link_demo_value.o
	$(CC) $^ -o $@

# Generic C compilation rule:
#   $< = source file
#   $@ = output object file
build/linux-native/%.o: build/linux-native/%.c
	$(CC) $(CFLAGS) -c $< -o $@
```

This achieves exactly the same build as the earlier versions. We earned the shortened syntax by first understanding every explicit command.

Try again:

```bash
make -n -B -f build/linux-native/Makefile
make -f build/linux-native/Makefile
```

## 7. Optional finishing touch: `all`, `clean`, and `.PHONY`

Some Make targets describe **actions** rather than files that should exist.

We can add a conventional `all` target and a narrowly scoped cleanup target:

```makefile
# Default goal (place this before other normal targets).
.PHONY: all clean

all: build/linux-native/link_demo

# Remove only the files generated by THIS experiment.
# Do not delete the whole build/linux-native directory:
# it also contains our GDB experiments and other files.
clean:
	rm -f build/linux-native/link_demo \
	      build/linux-native/link_demo_main.o \
	      build/linux-native/link_demo_value.o
```

Here:

- `all` depends on the executable but does not need a recipe of its own.
- `clean` runs a shell command that deletes only specified generated files.
- `.PHONY` marks `all` and `clean` as **action targets**, rather than filenames Make should check for existence.

If you add this, ensure `all` is the first ordinary target so it becomes the default goal. Run:

```bash
make -f build/linux-native/Makefile
make -f build/linux-native/Makefile clean
make -f build/linux-native/Makefile
```

The final command rebuilds the experiment after the cleanup.

**Caution:** This is still only a learning Makefile. Real projects should handle output directories and header dependencies carefully. The example assumes that the needed `build/linux-native` directory and experimental source files already exist.

## 8. Limits of our first dependency graph

So far, every object explicitly depends on its corresponding `.c` file. Later we will use shared `.h` headers.

If a header changes, a source file that includes it may need recompilation **even if its `.c` timestamp did not change**. A correct Makefile must include that dependency too.

For larger GCC projects, compiler-generated dependency files (often using `-MMD -MP`) can help keep those relationships accurate. We will study that when MemView gains real modules and headers, instead of introducing unnecessary complexity today.

There is another important boundary: `make` is a **build orchestrator**, not a C compiler. It decides which recipes to execute. GCC still performs compiling and linking.

## 9. Exercise: Predict before running

Before pressing Enter, predict what GNU Make will do in each situation:

| Situation | Expected action |
| --- | --- |
| Nothing has changed | No rebuilding needed |
| `link_demo_value.c` becomes newer than its object | Recompile `link_demo_value.o`, then relink |
| `link_demo_main.o` is deleted | Recompile the main object, then relink |
| The executable is deleted | Relink, using existing up-to-date object files |
| `make -n -B` is used | Print a plan for both compiling steps and linking |
| A relevant header changes but is not declared as a dependency | The simple Makefile may miss the required rebuild |

Confirm the predictions experimentally.

## 10. Applying GNU Make to the real MemView project

So far, our exercises have lived under `build/linux-native/`. That directory is intentionally ignored by Git. Now we are ready to create a **real, tracked Makefile** in the repository root.

The current MemView source tree contains only one C source file:

```text
MemView/
├── Docs/
├── pictures/
├── src/
│   └── main.c
├── build/             # ignored output and experiments
├── .gitignore
├── Makefile           # new, tracked project file
└── README.md
```

We will deliberately **start simpler** than our two-module linking experiment. GCC can compile and link one source file with a single command.

### 10.1 First working root Makefile

Create a file named `Makefile` (capital `M`) in the repository root:

```makefile
# MemView - Main Makefile

# C compiler
CC = gcc

# C17, no optimizations, debugging symbols, useful warnings
CFLAGS = -std=c17 -O0 -g -Wall -Wextra -pedantic

# The target is the path where the executable is created.
# $< = src/main.c (first prerequisite)
# $@ = build/memview (target)
build/memview: src/main.c
	mkdir -p build
	$(CC) $(CFLAGS) $< -o $@
```

GNU Make looks for a `Makefile` in the current directory, so there is no longer any need to provide `-f` when running from the repository root.

The **target name includes the directory** because `build/memview` is the actual file to create and whose modification time Make must check. Merely naming the target `memview` would not describe the same file.

Run:

```bash
make -n
make
./build/memview
echo $?
make
```

The program has `return 0;` in `main`, so the shell should show exit status `0`. The second invocation of `make` should have no reason to rebuild an up-to-date program.

**Important subtlety:** `mkdir -p build` is in the recipe, which Make runs only if the target needs updating. That works here. For more complex build graphs, we may use a dedicated directory target or other techniques to guarantee that output directories exist.

### 10.2 Introduce the default goal `all`

We can improve the Makefile using the action target `all`:

```makefile
# Default goal: build the application.
all: build/memview
```

If this is the **first ordinary target**, typing `make` chooses `all`. It has a prerequisite but does not need a recipe: Make follows the dependency to `build/memview`.

If the binary is already current, GNU Make does not recompile it merely because `all` was selected.

### 10.3 Introduce the cleanup goal `clean`

We also want a convenient way to remove the binary and test rebuilding from scratch:

```makefile
# Delete only the executable created by this Makefile.
clean:
	rm -f build/memview
```

Here `rm -f` ignores a nonexistent file and avoids a failure if `build/memview` is already absent.

**Safety rule:** Do not casually replace this with `rm -rf build`. Our `build/` directory contains other experimental source and assembly files, not all of which are outputs of this particular Makefile.

### 10.4 Why mark action targets as `.PHONY`?

Suppose someone creates a real file called `clean` in the repository root. Without additional information, Make might decide that the target already exists and skip the cleanup recipe.

The declaration

```makefile
.PHONY: all clean
```

marks `all` and `clean` as action targets rather than ordinary filenames.

This does **not** mean that the prerequisite `build/memview` must always be recompiled. It merely prevents Make from using the existence of files called `all` or `clean` to decide whether those action targets are complete.

### 10.5 The complete current MemView Makefile

After the steps above, our project-root `Makefile` becomes:

```makefile
# ==================================================
# MemView - Main Makefile
# ==================================================

# C compiler
CC = gcc

# Compiler options
# -std=c17 : use the C17 language standard
# -O0      : disable normal optimizations
# -g       : include debugging information
# -Wall    : enable common warnings
# -Wextra  : enable additional warnings
# -pedantic: diagnose certain nonconforming constructs
CFLAGS = -std=c17 -O0 -g -Wall -Wextra -pedantic

# These are action targets, not output filenames.
.PHONY: all clean

# Default goal: build MemView.
all: build/memview

# Build the executable from our first C source.
# $< = first prerequisite
# $@ = current target
build/memview: src/main.c
	mkdir -p build
	$(CC) $(CFLAGS) $< -o $@

# Remove only the output generated by this Makefile.
clean:
	rm -f build/memview
```

The indented recipe lines must begin with **literal TAB characters**, not spaces.

### 10.6 Test the entire cycle

From the repository root:

```bash
make -n
make clean
ls -l build/memview
make
./build/memview
echo $?
make
git status -sb
```

After `make clean`, `ls` should report that the executable does not exist. Running `make` creates it again. The program returns exit status `0`, and the final `make` should not need to rebuild it.

Since `build/` is listed in `.gitignore`, the generated executable is not normally staged. The root `Makefile`, however, is part of the project's source-controlled build instructions and **should** be committed once verified.

This is a practical example of why we separate **files we author** from **files our tools can regenerate**.

---

## 11. Summary

We improved **one build** in deliberate stages:

1. **Explicit linking rule:** distinguish target, prerequisites, and recipe.
2. **Complete explicit Makefile:** add the two separate compilation rules.
3. **Ordinary variables:** introduce `$(CC)` and `$(CFLAGS)`.
4. **Automatic variables:** use `$@`, `$<`, and `$^` to remove repeated filenames.
5. **Pattern rule:** replace two similar compilation rules with `%.o: %.c`.
6. **Action targets:** introduce `all`, `clean`, and `.PHONY` as a further step.

The most important principle is not remembering every special symbol. It is understanding **what the explicit rule meant before we abbreviated it**.

> **Documentation illustration:** The planned GNU Make cheat sheet will be stored as `pictures/gnu_makefile_cheat_sheet.png`. Until the image is committed, all examples remain fully explained in text.

Next we will return to the C language itself: types, objects, byte sizes, and the memory model. More elaborate Make rules will come later, when MemView has actual modules and tests.
