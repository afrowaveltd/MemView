# Lesson 02: How to Create a Project

## Learning Objectives

By the end of this lesson, you will understand:

- What a project directory is.
- How files and directories are organized.
- How to navigate the filesystem using a terminal.
- How to create a project without an IDE.
- How to create your first README file.
- Why a simple project structure is useful.

## 1. What Is a Software Project?

A software project is more than a collection of source code files.

It can contain:

- Source code written by programmers.
- Documentation describing the project.
- Configuration files for development tools.
- Automated tests.
- Build instructions.
- Resources required by the application.

A project does not necessarily require an Integrated Development Environment (IDE).

We can create, build and maintain a complete project using only a text editor, a compiler and a terminal.

**Important principle:**

An IDE helps us work with a project, but the project should not depend on a particular IDE.

## 2. Understanding Directories

A directory, also called a folder, is used to organize files and other directories.

Consider the following structure:

```text
Learning/
└── C/
    └── MemView/
        └── README.md
```

Here:

- `Learning` is a directory containing educational projects.
- `C` groups projects written in the C programming language.
- `MemView` is our project's root directory.
- `README.md` is a documentation file.

The project root is the main directory containing the project.

All future source code, tests and project documentation will be organized relative to this directory.

## 3. Understanding the Working Directory

When you open a terminal, the shell starts in a particular directory.

This is called the **current working directory**.

In Bash, enter:

```bash
pwd
```

The command `pwd` means *print working directory*.

Example output:

```text
/home/student
```

This tells us where we are currently working.

To display the contents of the current directory:

```bash
ls
```

To display additional information, including hidden files:

```bash
ls -la
```

The options have different meanings:

- `-l` displays a detailed listing.
- `-a` includes hidden files and directories.

On Unix-like systems, names beginning with a dot are normally hidden from a basic directory listing.

For example, Git commonly stores repository information in a hidden directory called `.git`.

## 4. Creating the Project Directory

We will now create a new directory for MemView.

The following commands use Bash on Linux or MSYS2.

First, navigate to the directory where you want to store your programming projects.

For example, on Linux:

```bash
mkdir -p ~/Learning/C
cd ~/Learning/C
```

Explanation:

- `mkdir` creates directories.
- `-p` also creates missing parent directories.
- `~` represents the current user's home directory.
- `cd` changes the current working directory.

Now create the MemView directory:

```bash
mkdir MemView
```

Enter it:

```bash
cd MemView
```

Verify your location:

```bash
pwd
```

Finally, inspect the directory:

```bash
ls -la
```

At this point, our project directory is empty.

That is exactly what we want.

We are starting from scratch.

## 5. Creating the First README File

Most software repositories contain a file named `README.md`.

It introduces the project and explains its purpose.

Open the terminal text editor:

```bash
nano README.md
```

If the file does not exist, Nano opens an empty buffer that can be saved as a new file.

Enter the following text:

```markdown
# MemView

MemView is an educational memory and hexadecimal viewer written in C.

## Project goals

- Learn the C programming language from scratch.
- Understand memory, pointers and data representation.
- Learn how compilers and linkers work.
- Build a terminal user interface.
- Support Linux and Windows.
- Practice Git and automated testing.
```

Save the document:

1. Press `Ctrl+O` to write the file.
2. Press `Enter` to confirm the filename.
3. Press `Ctrl+X` to exit Nano.

You have created your first project file without using an IDE.

## 6. Understanding Markdown

The `.md` extension identifies a Markdown document.

Markdown is a lightweight markup language used to format plain text.

For example:

```markdown
# Main heading

## Subheading

- First item
- Second item

**Bold text**
```

Markdown files remain readable in ordinary text editors.

Services such as GitHub can render Markdown into formatted documentation.

This makes Markdown especially useful for software projects.

## 7. Verify Your Work

Return to the terminal and enter:

```bash
ls -la
```

You should see your new file:

```text
README.md
```

You can display its contents without opening an editor:

```bash
cat README.md
```

The `cat` command writes the file's contents to standard output.

You should see the text you entered earlier.

## 8. Why Are We Starting Small?

It may seem tempting to create many directories immediately:

```text
src/
include/
tests/
build/
docs/
```

However, we have not written any C code yet.

Every directory should have a clear purpose.

We will introduce new directories when the project actually needs them.

This approach helps beginners understand why each part of the project exists.

## Summary

In this lesson, we learned how to:

- Navigate directories using a terminal.
- Create new directories.
- Identify the root of a project.
- Create and edit a README file.
- Understand basic Markdown syntax.
- Start a software project without an IDE.

In the next lesson, we will introduce Git and create our first commit.
