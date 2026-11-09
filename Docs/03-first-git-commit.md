# Lesson 03: Your First Git Commit

## Learning Objectives

By the end of this lesson, you will understand:

- What version control is and why we need it.
- The difference between Git and GitHub.
- How to initialize a Git repository.
- What the working tree and staging area are.
- How to create your first commit.
- How Git identifies commits.
- Why Git can work without an internet connection.

## 1. What Is Version Control?

Imagine that you are developing a program.

You write the first version, test it, and everything works.

The next day, you change several functions. Unfortunately, the program stops working.

How can you return to the previous working version?

One possibility is to create manual backup copies:

```text
MemView/
MemView-backup/
MemView-old/
MemView-final/
MemView-final-fixed/
```

This approach quickly becomes difficult to maintain.

Git provides a better solution.

Git is a distributed version control system that records the history of a project.

It allows us to:

- Record meaningful changes.
- Examine previous versions.
- Compare changes between versions.
- Experiment using separate branches.
- Collaborate with other developers.
- Restore previously committed content when necessary.

Git works locally and does not require a continuous internet connection.

## 2. Git Is Not GitHub

Git and GitHub are different things.

**Git** is the version control system installed on your computer.

**GitHub** is an online service that hosts Git repositories and provides collaboration tools.

You can use Git without GitHub.

You can also use other Git hosting services or maintain your own Git server.

This distinction is important because the history of your project does not belong exclusively to any particular hosting provider.

## 3. The Three Areas of Git

Git uses three important areas:

1. Working tree - files we are currently editing.
2. Staging area - content prepared for a commit.
3. Repository - committed project history.

The staging area is also called the Git index.

A file can have different content in the working tree
and in the staging area.

This allows us to choose exactly which changes
will become part of the next commit.


# Lesson 03: Your First Git Commit

## Learning Objectives

By the end of this lesson, you will understand:

- What version control is and why we need it.
- The difference between Git and GitHub.
- How to initialize a Git repository.
- What the working tree and staging area are.
- How to create your first commit.
- How Git identifies commits.
- Why Git can work without an internet connection.

## 1. What Is Version Control?

Imagine that you are developing a program.

You write the first version, test it, and everything works.

The next day, you change several functions. Unfortunately, the program stops working.

How can you return to the previous working version?

One possibility is to create manual backup copies:

```text
MemView/
MemView-backup/
MemView-old/
MemView-final/
MemView-final-fixed/
```

This approach quickly becomes difficult to maintain.

Git provides a better solution.

Git is a distributed version control system that records the history of a project.

It allows us to:

- Record meaningful changes.
- Examine previous versions.
- Compare changes between versions.
- Experiment using separate branches.
- Collaborate with other developers.
- Restore previously committed content when necessary.

Git works locally and does not require a continuous internet connection.

## 2. Git Is Not GitHub

Git and GitHub are different things.

**Git** is the version control system installed on your computer.

**GitHub** is an online service that hosts Git repositories and provides collaboration tools.

You can use Git without GitHub.

You can also use other Git hosting services or maintain your own Git server.

This distinction is important because the history of your project does not belong exclusively to any particular hosting provider.

## 3. The Three Areas of Git

Git uses three important areas:

1. Working tree - files we are currently editing.
2. Staging area - content prepared for a commit.
3. Repository - committed project history.

The staging area is also called the Git index.

A file can have different content in the working tree and in the staging area.

This allows us to choose exactly which changes will become part of the next commit.

## 4. Initialize a Git Repository

Open a terminal in the project root directory.

Run:

```bash
git init -b main
```

The command initializes a new Git repository.

- `init` creates the repository's internal structures.
- `-b main` names the initial branch `main`.

Git creates a hidden directory called `.git`.

This directory stores repository metadata, references, objects and other internal information.

Do not edit its contents manually while learning the basics.

Verify the repository:

```bash
git status
```

At this point, no commits exist.

## 5. Configure Your Identity

Git commits record an author's name and email address.

Check your current configuration:

```bash
git config user.name
git config user.email
```

If necessary, configure the current repository:

```bash
git config user.name "Your Name"
git config user.email "your-email@example.com"
```

Replace these example values with your chosen Git identity.

Using `--global` instead would configure the identity for repositories belonging to the current user, unless overridden.

For public repositories, consider using GitHub's private noreply email address.

## 6. Prepare Your First Commit

Assuming you created `README.md` in Lesson 02, run:

```bash
git status
```

Git should list `README.md` as an untracked file.

Add it to the staging area:

```bash
git add README.md
```

Check the result:

```bash
git status
```

The file should now appear under changes to be committed.

Inspect the staged changes:

```bash
git diff --cached
```

The `--cached` option compares the staging area with the current committed state. Before the first commit, Git compares against an empty initial state.

## 7. Create Your First Commit

Run:

```bash
git commit -m "Initial project"
```

The `-m` option supplies the commit message.

A commit records a snapshot of the staged project state, along with metadata such as the author, timestamp, message and parent references.

Now inspect the history:

```bash
git log --oneline
```

Git displays the new commit with an abbreviated identifier.

Check the repository again:

```bash
git status
```

If no further changes exist, the working tree should be clean.

**Congratulations! You have created your first Git commit without using an IDE.**

## 8. Mark a Milestone with a Tag

We can give an important historical commit a meaningful name:

```bash
git tag -a lesson-00-start -m "Initial project for beginners"
```

The `-a` option creates an annotated tag.

Unlike a branch, a tag does not automatically move forward when new commits are created.

List the tags:

```bash
git tag -n
```

## 9. Publish Your Project to GitHub

Create a new empty repository on GitHub.

Do not initialize it with another README if you already created your first commit locally.

Connect the local repository using the URL of your own GitHub repository:

```bash
git remote add origin https://github.com/OWNER/MemView.git
```

Replace `OWNER` with your GitHub username or organization.

Verify the remote configuration:

```bash
git remote -v
```

Publish the main branch:

```bash
git push -u origin main
```

The `-u` option configures the upstream tracking relationship.

Publish the tag separately:

```bash
git push origin lesson-00-start
```

Ordinary branch pushes do not automatically publish all local tags.

### GitHub Authentication

GitHub does not accept an account password for Git operations over HTTPS.

Use a supported authentication method, such as:

- Git Credential Manager.
- An SSH key.
- A personal access token.

Two-factor authentication should remain enabled.

Never commit passwords, authentication tokens or private SSH keys.

## 10. Inspect Your History

Run:

```bash
git log --oneline --decorate --graph --all
```

This displays a compact view of commits, branches and available references.

You can also inspect the files recorded in the current commit:

```bash
git ls-tree --name-only HEAD
```

`HEAD` normally refers to the currently checked-out commit through the active branch.

Git stores file contents in objects called blobs, directory structures in trees, and historical snapshots in commits.

We will explore these objects more deeply in a future lesson.

## Summary

You have learned how to initialize a repository, stage files, create commits, inspect history, add tags and publish your work.

The important principle is that Git works locally. GitHub is an optional remote location for sharing the repository.

**Understand what you are recording before committing it.**
