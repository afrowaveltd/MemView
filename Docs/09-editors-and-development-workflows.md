# Lesson 09: Configure Your Editor and Development Workflow

> **Living laboratory:** The same MemView sources should build correctly regardless of the chosen editor. This chapter records tested configurations, problems, and solutions. Planned experiments are clearly marked.

## Goals

Understand the difference between an editor, language server, compiler, build tool, linker, and debugger. Configure VS Code to use our existing **GNU Make + GCC + C17** toolchain, diagnose false errors, and plan equivalent checks for other editors on Linux and Windows.

For initial installations, read [Lesson 01](01-setup-development-environment.md). For modules, includes, and linking, read [Lesson 08](08-modules-headers-and-separate-compilation.md).

## 1. One project, several different tools

| Component | Responsibility | Example in MemView |
| --- | --- | --- |
| Text editor | Write and save files | nano, mcedit, VS Code |
| Language server | Completion, navigation, diagnostics while typing | Microsoft C/C++, clangd |
| Compiler | Translate C to object code | GCC |
| Build system | Decide what to compile and link | GNU Make |
| Linker | Resolve symbols across object files | Invoked by GCC |
| Debugger | Inspect execution, variables, and memory | GDB |
| Version control | Track project history | Git |

**The root `Makefile` is our source of truth for building MemView.** The editor should cooperate with it, not silently create a separate incompatible build.

Our actual configuration:

```makefile
CC = gcc
CPPFLAGS = -Iinclude
CFLAGS = -std=c17 -O0 -g -Wall -Wextra -pedantic
```

We use:

```text
MemView/
├── include/
│   └── helpers.h
├── src/
│   ├── main.c
│   └── helpers.c
├── build/                 # Generated; not tracked
├── .vscode/               # Optional editor-specific settings
└── Makefile
```

## 2. Before changing anything: identify who reported the error

An editor can underline valid code if its parser does not know the actual compiler arguments. Equally, an editor can look clean even if the real program does not link or run.

Our troubleshooting method:

1. Run `make` in a terminal at the repository root. Is the *real GCC build* failing?
2. Open the editor's diagnostic list and read the **exact** error and its source.
3. Identify which language server is active. Is more than one parsing the same source?
4. Confirm the correct compiler, C language version, and header search path.
5. Fix the appropriate configuration, rather than changing valid C code or hiding all warnings.
6. Rebuild and reopen the source to verify the result.

Common symptoms:

| Symptom | Likely area to check |
| --- | --- |
| `'helpers.h' file not found`, but Make succeeds | Language server lacks `-Iinclude` |
| `undefined reference to 'set_bit'` | Linking omitted `helpers.o` |
| `gcc: command not found` | Wrong shell or `PATH` |
| False diagnostics remain after configuring Microsoft C/C++ | Another parser, such as clangd, may still be active |
| C file interpreted as C++ | File language mode or toolchain configuration |
| Object/executable files fail after switching operating systems | Clean platform-specific build artifacts |

Our first command-line checkpoint on Linux is:

```bash
make clean
make
./build/memview
```

## 3. VERIFIED: Debian 13, GCC 14.2, VS Code, Makefile Tools

**Tested with MemView: 11 November 2026.** The program compiled correctly throughout; the remaining false squiggle was fixed by disabling an independently configured clangd language server for this workspace.

### 3.1 Open the project folder

In the Debian terminal:

```bash
cd /mnt/PROGRAMMING/Learning/C/MemView
code .
```

Your own checkout path may differ. Opening the entire folder is important because workspace and Makefile settings are relative to the root.

We used these VS Code extensions:

- **C/C++** by Microsoft — `ms-vscode.cpptools`.
- **Makefile Tools** by Microsoft — `ms-vscode.makefile-tools`.

We also had **clangd** installed. That is a legitimate alternative language server, but it has its own configuration; it is not automatically fed the Microsoft extension's settings.

### 3.2 Manual IntelliSense settings

When using Microsoft C/C++ without a provider, you can create `.vscode/c_cpp_properties.json`:

```json
{
    "configurations": [
        {
            "name": "MemView - Debian GCC",
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c17",
            "intelliSenseMode": "linux-gcc-x64",
            "includePath": [
                "${workspaceFolder}/include",
                "${workspaceFolder}/src"
            ]
        }
    ],
    "version": 4
}
```

**Important nuance:** A configuration provider such as Makefile Tools can take precedence over some manual settings and supply more accurate *per-source-file* build settings. Having a provider is not an error.

The actual diagnostic output from our Debian setup included:

```text
configurationProvider: ms-vscode.makefile-tools
compilerPath:          /usr/bin/gcc
Standard Version:      c17
IntelliSense Mode:     linux-gcc-x64
Include Paths:         .../MemView/include
GCC version:           14.2.0
```

That proved **Microsoft IntelliSense already had the correct settings**. Repeatedly changing `c_cpp_properties.json` could not solve a problem originating from another extension.

References: [Official VS Code IntelliSense guide](https://code.visualstudio.com/docs/cpp/configure-intellisense), [C/C++ extension settings](https://code.visualstudio.com/docs/cpp/customize-cpp-settings).

### 3.3 Build with Make, not a single-file shortcut

To use **Ctrl+Shift+B** for the complete project, create `.vscode/tasks.json`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build MemView",
            "type": "shell",
            "command": "make",
            "options": { "cwd": "${workspaceFolder}" },
            "group": { "kind": "build", "isDefault": true },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "Clean MemView",
            "type": "shell",
            "command": "make",
            "args": ["clean"],
            "options": { "cwd": "${workspaceFolder}" },
            "problemMatcher": []
        }
    ]
}
```

Why not `gcc src/main.c -o main`? Because that command would omit the implementation in `src/helpers.c` and leave unresolved function references.

**Building via tasks and providing IntelliSense configuration are separate concerns.** One may work while the other does not.

### 3.4 Our actual false-squiggle incident

**Symptom:** In `src/helpers.c` the editor underlined:

```c
#include "helpers.h"
```

with an error equivalent to:

```text
'helpers.h' file not found
```

**Evidence:** The `C/C++: Log Diagnostics` output showed GCC 14.2, C17, and the correct `include/` path. Yet the squiggle remained. The VS Code status bar also indicated **clangd** was running.

**Diagnosis:** Two independent language-server systems were active. Microsoft C/C++ had configuration supplied by Makefile Tools. clangd did not automatically receive that configuration and incorrectly reported a missing header.

**Verified fix:**

1. Open VS Code Extensions with **Ctrl+Shift+X**.
2. Find the **clangd** extension.
3. Select **Disable (Workspace)**; do not uninstall it globally.
4. Open the Command Palette with **Ctrl+Shift+P**.
5. Run **Developer: Reload Window**.
6. Reopen the C file.

**Observed result:** The false squiggle disappeared. No changes to the valid `#include "helpers.h"` directive or the working Makefile were necessary.

**Rule:** First find out *which diagnostic engine* emitted the error. Do not change correct code to satisfy the wrong parser.

### 3.5 Diagnostic commands worth remembering

Within VS Code:

- **Ctrl+Shift+M** — Problems; read the exact diagnostic message and its originating extension.
- **Ctrl+Shift+P**, then **C/C++: Log Diagnostics** — view active compiler, include paths, language standard, and configuration provider.
- **Ctrl+Shift+P**, then **C/C++: Select IntelliSense Configuration** — select the intended configuration/provider.
- **Ctrl+Shift+P**, then **C/C++: Reset IntelliSense Database** — useful for stale Microsoft IntelliSense state *after* checking settings, not a substitute for fixing a missing path.
- **Ctrl+Shift+B** — run the default build task, if `tasks.json` has been configured.

Checkpoint: `make` succeeds, `helpers.h` resolves, the editor does not show false missing-header errors, and the actual project builds from both terminal and task.

## 4. Alternative: configure clangd deliberately

**Status: documented option, not yet our tested setup.**

There is no requirement to abandon clangd permanently. clangd reads compile flags independently and can use a compilation database named `compile_commands.json`.

On Linux, a tool such as **Bear** can record build commands during a full build:

```bash
# Only when Bear is installed and you choose to use clangd.
make clean
bear -- make
```

Why `make clean`? An up-to-date build might not invoke any compiler, leaving nothing for Bear to record.

Generated compilation databases contain paths/toolchain details that are not always transferable between operating systems. Another approach is project configuration via a `.clangd` YAML file; for a project whose compile command runs from the repository root, one candidate is:

```yaml
CompileFlags:
  Add: [-std=c17, -Iinclude]
```

This is a **candidate to test**, not an assurance that relative paths and all system includes are correct in every workflow.

See [clangd installation and project setup](https://clangd.llvm.org/installation) and [clangd configuration](https://clangd.llvm.org/config).

For our verified Debian setup, **Microsoft C/C++ + Makefile Tools** is the simpler approach.

## 5. Terminal-first editors: nano and mcedit

**Status: our terminal editing and GCC/Make build workflow is in use.**

```bash
nano src/main.c
nano include/helpers.h
make
./build/memview
```

Or:

```bash
mcedit src/helpers.c
```

There is no separate IntelliSense parser to configure. The trade-off is fewer automatic code-analysis features, but a direct and predictable relationship between the editor and compiler.

These tools are useful baselines when a graphical IDE behaves strangely. If GCC reports no error in a terminal, diagnose the IDE before rewriting the program.

## 6. Windows: MSYS2 UCRT64 + VS Code

**Status: planned joint verification on native Windows.** Lesson 01 covers toolchain installation; the complete MemView editor integration below is a **test plan**, not yet an experimentally verified result.

Begin in the **MSYS2 UCRT64** terminal:

```bash
echo "$MSYSTEM"
command -v gcc
gcc --version
gcc -dumpmachine
command -v make
make --version
```

Expected characteristics: `MSYSTEM` reports `UCRT64` and GCC commonly lives in `/ucrt64/bin/gcc`.

Then test:

1. Build the repository directly in UCRT64 with `make clean && make`.
2. Verify which native Windows executable is generated and how to run it.
3. Open the repository folder in VS Code.
4. Verify that its **integrated terminal** resolves the same compiler and Make executable.
5. Configure the correct Windows GCC/MinGW IntelliSense mode and header search path.
6. Confirm the build task includes both `main.o` and `helpers.o` through the Makefile.
7. Test debugging using the corresponding Windows GDB.
8. Document precise versions, screenshots, problems, fixes, and results.

**Do not copy Linux values** such as `/usr/bin/gcc` and `linux-gcc-x64` into a native Windows IntelliSense configuration.

**Toolchain isolation matters.** If the same checkout is used alternately for native Windows and Linux, do `make clean` before switching. Native object files and executables are not interchangeable. In a later lesson we can introduce separate output folders per platform.

VS Code is not Visual Studio; they are different products with different project models.

## 7. WSL: Linux toolchain accessed from Windows

**Status: planned separate editor test.**

Within WSL:

```bash
command -v gcc
gcc -dumpmachine
make --version
```

When using a VS Code WSL workflow, check that the language server, compiler, and build task belong to the **Linux environment**, not accidentally the native Windows MinGW installation.

We will verify project opening, include paths, build tasks, run, and debugging in WSL and record the actual results.

## 8. Other editors and IDEs: follow the same test checklist

**Not yet fully verified with this MemView layout.**

| Editor / IDE | Proposed environment | What we want to prove |
| --- | --- | --- |
| **Code::Blocks** | Debian and Windows | Calls the intended compiler/Makefile; resolves headers |
| **CLion** | Debian and Windows | Opens our Makefile project and uses the selected toolchain correctly |
| **Visual Studio** | Windows | Explicitly distinguish an MSVC build from our GCC/Make build |
| **VS Code + clangd** | Debian and Windows | Reads accurate per-file compiler commands |
| **nano / mcedit** | Linux, WSL, MSYS2 where available | Terminal build gives consistent results |

CLion has [documented Makefile project support](https://www.jetbrains.com/help/clion/makefiles-support.html), but its documentation notes limitations. We should test the project instead of claiming complete support.

For every test, record editor/OS version, GCC path and version, `gcc -dumpmachine`, standard, build command, include paths, debugger, reproducible errors, and verified fixes.

### Record format for future sessions

```text
Environment:
Editor / version:
Compiler and target:
Build command:
Source/headers detected:
Run/debug result:
Problem observed:
Cause:
Fix:
Verification status: Tested / Needs retest / Planned
```

A reproducible problem and its fix are valuable teaching material. Avoid deleting the failed step from the documentation: explain what was wrong and how we knew.

## 9. Should editor settings be committed to Git?

VS Code may generate:

```text
.vscode/
├── c_cpp_properties.json
├── tasks.json
├── launch.json
└── settings.json
```

Some files are helpful across the team; others contain absolute paths, private preferences, or assumptions specific to a machine.

Inspect before staging:

```bash
git status --short
git diff --check
```

Decide deliberately what should be shared. The Makefile remains authoritative.

## 10. Verification log

| Configuration | Status | Notes |
| --- | --- | --- |
| Debian 13 + GCC 14.2 + Make | **Verified** | Compiles with `-std=c17` and `-Iinclude` |
| Debian 13 + VS Code + Microsoft C/C++ + Makefile Tools | **Verified** | Compiler, C17, header paths diagnosed correctly |
| Debian 13 + VS Code with both C/C++ and clangd | **Issue resolved** | clangd false missing-header diagnostic fixed by disabling it per workspace |
| Native Windows + MSYS2 UCRT64 + VS Code | **Planned** | To test together |
| WSL + VS Code | **Planned** | To test together |
| Code::Blocks and CLion | **Planned** | Makefile integration to test |
| Windows Visual Studio | **Planned** | Separate MSVC/toolchain investigation |

**Main principle:** A red squiggle is a diagnostic from one tool, not a verdict from the C compiler. Identify the tool and reproduce the real build before changing source code.
