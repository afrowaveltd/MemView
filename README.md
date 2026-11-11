# MemView

**MemView** is an educational memory and hexadecimal viewer written in C.

The project is developed step by step as a practical course in C, memory representation, compilers, debuggers, Git, and terminal applications. **It is a learning project in progress, not yet a finished memory viewer.**

## Learning objectives

- Learn the C programming language from the fundamentals.
- Understand addresses, pointers, data types, the stack, and binary representation.
- Investigate preprocessing, compilation, assembly, linking, and execution.
- Build a portable terminal application with a clean separation between its logic and UI.
- Support Linux and Windows, with **ncurses** and **PDCurses** as planned terminal backends.
- Develop good habits in testing, debugging, version control, and documentation.

## Lessons

The lessons are written in English, with reproducible terminal commands and explanations of *why* each step matters.

1. [Set Up the Development Environment](Docs/01-setup-development-environment.md) — toolchains, terminals, editors, GCC, GDB, and Git.
2. [Create a Project](Docs/02-create-project.md) — directories, source files, and Markdown.
3. [Your First Git Commit](Docs/03-first-git-commit.md) — staging, commits, tags, and GitHub.
4. [Our First C Program](Docs/04-first-c-program.md) — preprocessing, assembly, object files, linking, GDB, and memory.
5. [GNU Make](Docs/05-gnu-make.md) — incremental builds, explicit rules, ordinary variables, automatic variables, and pattern rules.
6. [C Integer Types, Bytes, and Signedness](Docs/06-c-integer-types.md) — `sizeof`, character constants, integer promotions, unsigned wraparound, and signed overflow.
7. [Bitwise Operations, Shifts, and Masks](Docs/07-bitwise-operations-and-masks.md) — AND, OR, XOR, NOT, bit shifts, and reading individual bits.
8. [Functions, Headers, and Separate Compilation](Docs/08-modules-headers-and-separate-compilation.md) — modules, include guards, function declarations, and linking object files.
9. [Configure Your Editor and Development Workflow](Docs/09-editors-and-development-workflows.md) — verified VS Code/Make setup, IntelliSense vs clangd, troubleshooting, and upcoming Windows/editor tests.
10. [Our First Automated Tests in C](Docs/10-first-automated-c-tests.md) — assertions, separate test executables, deliberate failures, and a `make test` target.
11. [Loops in C and Our First Binary Display](Docs/11-loops-and-binary-output.md) — `while`, `do ... while`, `for`, `break`, `continue`, unsigned loop traps, and printing a byte in binary.
12. [Arrays, Contiguous Memory, and sizeof](Docs/12-arrays-memory-layout-and-sizeof.md) — array layout, byte sizes, `size_t`, indexing, bounds, and array-versus-pointer behavior.
13. [Nested Loops and Our First Multi-Byte Display](Docs/13-nested-loops-and-byte-display.md) — combine arrays, reusable bit helpers, and nested loops into hex/binary byte output.

The [pictures](pictures/) directory contains diagrams and quick-reference illustrations used in the lessons.

## Repository layout

```text
MemView/
├── Docs/           # English learning materials
├── include/        # Public module headers
│   └── helpers.h
├── pictures/       # Illustrations and reference sheets
├── src/            # C implementations and main program
│   ├── main.c
│   └── helpers.c
├── tests/          # Automated C tests
│   └── test_helpers.c
├── Makefile        # Compile, link, and run tests
├── README.md
└── .gitignore
```

Generated files and local build experiments belong under `build/` and are not committed.

## Learning approach

**Understand the theory → inspect the mechanism → implement → test → document.**

The repository intentionally grows in small, verifiable steps. A simple program that returns zero is useful when it helps us understand precisely what the compiler and processor are doing.

## License

No license is declared in this repository yet. Do not assume that the absence of a license grants permission to redistribute or modify its contents outside the permissions provided by applicable law.
