# Lesson 01: Setting Up the Development Environment

## Learning objectives

By the end of this lesson, you will understand:

- What a development environment is.
- The difference between an editor, compiler and debugger.
- What a terminal and shell are.
- How to prepare Windows and Linux for C development.
- How to verify that the required tools are working.

## Introduction

Before writing our first C program, we need to
understand the tools used to create software.

A development environment is not just an IDE.
It consists of several independent tools that
work together to transform source code into
a running application.

For editor-specific configuration, verified troubleshooting cases, and cross-platform IDE testing, see [Lesson 09: Configure Your Editor and Development Workflow](09-editors-and-development-workflows.md).

## Understanding the Development Toolchain

A development environment consists of several tools. Each tool has a different responsibility.

### 1. Text editor

A text editor allows us to write and modify source code.

Examples include:

- nano
- Visual Studio Code
- Code::Blocks
- CLion
- Visual Studio

An editor does not have to understand how to translate C into machine code. Its primary responsibility is working with text.

### 2. Compiler

A compiler translates source code into a representation that can ultimately become machine instructions.

In this project, we will primarily use GCC (GNU Compiler Collection).

### 3. Assembler

An assembler converts assembly instructions into object code.

Object files usually contain machine instructions, symbols and information required by the linker.

### 4. Linker

A linker combines object files and resolves references to functions and other symbols.

It also connects our program with the libraries it requires.

### 5. Debugger

A debugger helps us investigate a program while it is running.

We can use it to:

- Pause program execution.
- Inspect variables and memory.
- Examine processor registers.
- Execute instructions step by step.
- Investigate unexpected behaviour.

We will use GDB on Linux and MinGW, and explore the Visual Studio debugger on Windows.

### 6. Operating system

The operating system provides the environment in which the compiled application runs.

It manages processes, virtual memory, files and access to hardware resources.

A compiled program is loaded and executed according to the rules of its target operating system.

## Important Principle

**An IDE is not the same thing as a compiler.**

An IDE may combine an editor, compiler integration, debugger and build tools into one interface, but these components remain conceptually separate.

## Setting Up C Development on Windows

### 1. Installing MSYS2

Windows does not include GCC as a standard component.

For this project, we will use MSYS2, a development environment that provides compilers, command-line tools, libraries and a package manager.

Download MSYS2 from its official website:

https://www.msys2.org/

Install MSYS2 and open the **MSYS2 UCRT64** terminal.

UCRT64 provides a GCC toolchain for building native 64-bit Windows applications using the Universal C Runtime.

### 2. Updating MSYS2

Before installing development tools, update the package database and installed packages:

```bash
pacman -Syu
```

If MSYS2 asks you to close the terminal during an update, reopen UCRT64 and run the command again.

### 3. Installing the Development Tools

Install GCC, GDB, Make, Git and nano:

```bash
pacman -S --needed \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-gdb \
    mingw-w64-ucrt-x86_64-git \
    make \
    nano
```

These tools have different purposes:

- **GCC:** Compiles C source code.
- **GDB:** Debugs compiled programs.
- **Make:** Automates building projects.
- **Git:** Tracks changes to source code.
- **nano:** Edits text files in the terminal.

### 4. Checking the Installation

Run the following commands:

```bash
gcc --version
gdb --version
make --version
git --version
```

Each command should display information about the installed program.

The exact version numbers may differ.

### 5. Identifying the Compiler

Run:

```bash
gcc -dumpmachine
```

For 64-bit Windows using MinGW-w64, a typical result is:

```text
x86_64-w64-mingw32
```

This identifier describes the target platform for which GCC generates code.

To find the executable used by the shell:

```bash
command -v gcc
```

In MSYS2 UCRT64, the result should normally be:

```text
/ucrt64/bin/gcc
```

### 6. Understanding the PATH

The PATH environment variable contains directories in which the shell searches for executable programs.

Display it using:

```bash
echo "$PATH"
```

A computer can have multiple GCC installations. The compiler selected depends on how the current shell resolves the command.

**Important:** Use the UCRT64 terminal when building with UCRT64 libraries. Avoid accidentally mixing libraries from different toolchains.

### 7. Windows and MSYS2 Paths

Windows commonly uses paths such as:

```text
C:\Projects\MemView
```

MSYS2 provides Unix-style paths:

```text
/c/Projects/MemView
```

Both paths can refer to the same directory.

### Checkpoint

Before proceeding, verify that:

- GCC is available.
- GDB is available.
- Make is available.
- Git is available.
- The active MSYS2 environment is UCRT64.
- You can navigate directories using the terminal.

You now have the basic tools required to begin developing C programs on Windows.


## Setting Up Debian Linux

We will use Debian GNU/Linux as one of our development environments.

The following instructions were tested with Debian 13 (Trixie), including Debian running under Windows Subsystem for Linux (WSL).

### Step 1: Update the package index

Open a terminal and enter:

```bash
sudo apt update
```

**Explanation:**

- `sudo` executes a command with elevated privileges, normally as the root user.
- `apt` is Debian's package management tool.
- `update` downloads information about available packages from configured repositories.

This command does not upgrade all installed applications. It refreshes the package information used by APT.

### Step 2: Install development tools

```bash
sudo apt install build-essential gdb git nano
```

We install four packages:

- `build-essential`: A collection of essential development tools and dependencies, including GCC and GNU Make.
- `gdb`: The GNU Debugger, used to inspect programs during execution.
- `git`: A distributed version control system.
- `nano`: A terminal-based text editor.

The `build-essential` package provides a convenient way to install the basic C development toolchain on Debian.

### Step 3: Verify the installation

Check the compiler:

```bash
gcc --version
gcc -dumpmachine
```

Check the build tool:

```bash
make --version
```

Check the debugger:

```bash
gdb --version
```

Check Git:

```bash
git --version
```

Check the editor:

```bash
nano --version
```

If these commands display their respective version information without errors, the basic development environment is ready.

### Step 4: Understand the target platform

Run:

```bash
gcc -dumpmachine
```

On a typical 64-bit Debian system using the x86-64 architecture, the result is:

```text
x86_64-linux-gnu
```

This identifier describes the compiler's target platform.

A Windows compiler may instead report:

```text
x86_64-w64-mingw32
```

Both compilers can process C source code, but they normally produce executable files for different operating systems.

**Important:** Portable C source code is not the same thing as a portable compiled executable.

### Development in WSL

Windows Subsystem for Linux allows us to run a Linux development environment alongside Windows.

Windows drives are commonly accessible through paths such as:

```text
/mnt/c/
/mnt/d/
```

For example, a Windows project stored in `C:\Projects\MemView` can usually be accessed from WSL through:

```bash
cd /mnt/c/Projects/MemView
```

When building the same project for Windows and Linux, generated object files and executables should be kept in separate output directories.

This prevents incompatible build artifacts from being mixed together.

## Setting Up Windows Development

Windows supports several ways to develop applications in C.

For this project, we will explore:

- MSYS2 with the MinGW-w64 UCRT64 toolchain.
- Standalone MinGW-w64.
- Microsoft Visual Studio with its C/C++ development tools.

We recommend MSYS2 UCRT64 for beginners following the command-line examples in this tutorial.

### Step 1: Install MSYS2

Visit the official website:

https://www.msys2.org/

Download and install MSYS2 using the official installation instructions.

MSYS2 provides a Unix-like command-line environment for Windows, along with a package manager called Pacman.

**Important:** MSYS2 is not a Linux virtual machine. It provides tools that run on Windows.

### Step 2: Open the UCRT64 Environment

Launch the **MSYS2 UCRT64** terminal.

Verify the selected environment:

```bash
echo $MSYSTEM
```

Expected result:

```text
UCRT64
```

The `$MSYSTEM` environment variable identifies the currently selected MSYS2 environment.

Different MSYS2 environments can have different compilers, libraries and runtime dependencies.

### Step 3: Update MSYS2

Run:

```bash
pacman -Syu
```

Pacman is the package manager used by MSYS2.

The options mean:

- `-S`: Synchronize packages.
- `-y`: Refresh package databases.
- `-u`: Upgrade installed packages.

If the update requests a terminal restart, close MSYS2, reopen UCRT64 and complete the update.

### Step 4: Install Development Tools

Run:

```bash
pacman -S --needed \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-gdb \
    mingw-w64-ucrt-x86_64-git \
    make \
    nano
```

These packages provide the compiler, debugger, Git, build automation and a terminal editor.

The `--needed` option skips packages that are already installed and up to date.

The backslash at the end of a command line allows the Bash command to continue on the following line.

### Step 5: Verify GCC

Run:

```bash
gcc --version
gcc -dumpmachine
command -v gcc
```

On an x86-64 UCRT64 installation, the target commonly appears as:

```text
x86_64-w64-mingw32
```

The GCC executable should normally be found at:

```text
/ucrt64/bin/gcc
```

These commands help us identify both the compiler version and the compiler actually selected by the shell.

### Step 6: Verify the Remaining Tools

```bash
make --version
gdb --version
git --version
nano --version
```

Each command should report its version information.

If a command is not found, verify that the corresponding package is installed and the correct MSYS2 environment is active.

### Step 7: Understand Windows Paths

MSYS2 uses Unix-style paths.

For example:

| Windows path | MSYS2 path |
|---|---|
| `C:\Projects` | `/c/Projects` |
| `D:\Development` | `/d/Development` |
| `L:\Learning\C\MemView` | `/l/Learning/C/MemView` |

To open our project from MSYS2:

```bash
cd /l/Learning/C/MemView
```

The drive must exist and be accessible to the current user.

### Using Standalone MinGW-w64

MSYS2 is not the only way to use GCC on Windows.

A standalone MinGW-w64 toolchain can also compile C programs into native Windows executables.

In PowerShell, run:

```powershell
gcc --version
gcc -dumpmachine
(Get-Command gcc).Source
```

The last command shows the executable selected by PowerShell.

For example:

```text
C:\mingw64\bin\gcc.exe
```

Two installations can report the same GCC version while using different build configurations, runtime libraries or supporting tools.

**Important:** Avoid mixing object files and libraries from incompatible toolchains. Keep build outputs separate.

### Git and GitHub Authentication

Git can be used locally without a GitHub account or an internet connection.

Publishing changes to GitHub requires authentication.

GitHub supports methods such as SSH keys and HTTPS authentication through a credential manager or personal access token.

An account using two-factor authentication (2FA) can still use Git normally.

If Git works in PowerShell but authentication fails in MSYS2, the two environments may use different credential-helper configurations.

Never store passwords, access tokens or private SSH keys in the project repository.

### Summary

We have learned that:

- Windows can run several independent C toolchains.
- MSYS2 provides a convenient package-managed development environment.
- UCRT64 can build native Windows applications.
- The shell uses its environment and PATH configuration to locate programs.
- A compiler version alone does not identify the entire toolchain.
- The same C source code can be compiled for different operating systems.

In the next lesson, we will create our project structure and begin exploring the compilation process.
