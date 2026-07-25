*This project has been created as part of the 42 curriculum by Visual, Daniela.*

# minishell

**Summary:** A minimal shell that reproduces a subset of `bash`: command execution, pipes, redirections, quoting, environment-variable expansion, signals, and seven built-ins.

<br>

## Table of Contents

- [Description](#description)
- [Instructions](#instructions)
- [How It Works](#how-it-works)
- [Features](#features)
- [Project Structure](#project-structure)
- [Documentation](#documentation)
- [Testing](#testing)
- [Resources](#resources)
- [AI Usage](#ai-usage)

<br>

## Description

minishell is a command-line interpreter written in C. It displays a prompt, reads a line, and runs it the way `bash` would: resolving executables through `PATH`, wiring commands together with pipes, applying redirections, expanding variables, and reporting the exit status of the last foreground command.

When in doubt about a behaviour, `bash` is the reference. Anything the subject does not require (backslash escaping, `;`, `&`, wildcards) is deliberately left uninterpreted.

<br>

## Instructions

```bash
make        # build libft, then minishell
make clean  # remove object files
make fclean # remove object files and the binary
make re     # full rebuild
```

```bash
./minishell
minishell$ ls -la | grep .c | wc -l
minishell$ echo "hello $USER" > out.txt
minishell$ cat << EOF
```

`readline` is required to build (`-lreadline`).

<br>

## How It Works

A line travels through five stages before anything runs. Each stage has one job and hands a cleaner structure to the next.

```
  input line
      │
   ┌──▼───────┐ splits the raw line into tokens (words, |, <, >, <<, >>),
   │ LEXER    │ tracks quote state, flags unclosed quotes
   └──┬───────┘
   ┌──▼───────┐ groups tokens into a command list, attaches redirections,
   │ PARSER   │ reports syntax errors (status 2)
   └──┬───────┘
   ┌──▼───────┐ resolves $VAR and $?, strips quotes, the single owner of
   │ EXPANDER │ all quote handling in the project
   └──┬───────┘
   ┌──▼───────┐ reads heredoc bodies before execution, expanding unless the
   │ HEREDOC  │ delimiter was quoted
   └──┬───────┘
   ┌──▼───────┐ forks the pipeline, applies redirections, runs built-ins in
   │ EXECUTOR │ the parent when the line is a single built-in
   └──┬───────┘
      ▼
  exit status → $?
```

Each module has its own `README.md` under `src/` explaining its internals in more detail.

**Signals.** A single global (`volatile sig_atomic_t g_signal`) stores nothing but the number of a received signal, as the subject requires. The handler leaves a breadcrumb; the main loop reads it to set `$?` to 130 after `Ctrl-C`. At the prompt, `Ctrl-C` redraws a fresh line, `Ctrl-\` is ignored, and `Ctrl-D` exits.

<br>

## Features

**Built-ins:** `echo` (with `-n`), `cd`, `pwd`, `export`, `unset`, `env`, `exit`.

**Redirections:** `<`, `>`, `>>`, and `<<` (heredoc, no history).

**Pipes:** arbitrary-length pipelines (`cmd1 | cmd2 | cmd3`).

**Quoting:** single quotes suppress all interpretation; double quotes suppress everything except `$`.

**Expansion:** `$VAR` from the environment and `$?` for the last exit status, quote-aware.

**Path resolution:** absolute, relative, and `PATH` lookup (left to right).

<br>

## Project Structure

```
src/
├── main.c         entry point: argc check, init, run loop, free
├── shell_init.c   build the shell struct, copy the environment, install signals
├── shell_loop.c   read-eval loop: readline, history, run each line
├── lexer/         raw line → token list
├── parser/        token list → command list + redirections
├── expander/      $VAR / $? expansion and quote removal
├── heredoc/       << body reading and expansion
├── executor/      pipelines, redirections, path resolution, built-in dispatch
├── builtins/      echo, cd, pwd, export, unset, env, exit
├── env/           environment array: init, get, set, unset
├── signals/       prompt / child / heredoc signal modes
└── utils/         error printing, memory cleanup
```

<br>

## Documentation

Every module carries its own README explaining how it works, with code snippets
and the tricky parts spelled out. Start with **[`src/`](src/README.md)**, which
maps the whole pipeline and suggests a reading order for learning the project
from scratch.

- [`src/`](src/README.md): architecture overview and reading order
- [`includes/`](includes/README.md): the four core structs and the data model
- [`src/lexer/`](src/lexer/README.md) · [`src/parser/`](src/parser/README.md) · [`src/expander/`](src/expander/README.md) · [`src/heredoc/`](src/heredoc/README.md): turning text into commands
- [`src/executor/`](src/executor/README.md): fork, pipes, redirections, dispatch
- [`src/builtins/`](src/builtins/README.md) · [`src/env/`](src/env/README.md) · [`src/signals/`](src/signals/README.md) · [`src/utils/`](src/utils/README.md): built-ins and support layers

<br>

## Testing

Behaviour is validated by diffing minishell against `bash` on the same input, plus `valgrind` for leaks and `norminette` for style.

```bash
norminette includes/ src/
printf 'ls | grep .c | wc -l\nexit\n' | valgrind --leak-check=full ./minishell
```

The only tolerated leaks are those internal to `readline`, which the subject exempts. All code we wrote is leak-free.

<br>

## Resources

- [42 Cursus Gitbook: minishell](https://web.archive.org/web/20250306211139/https://42-cursus.gitbook.io/guide/rank-03/minishell) (original site offline; linked via the Wayback Machine)
- [Handling a File by its Descriptor in C (CodeQuoi)](https://www.codequoi.com/en/handling-a-file-by-its-descriptor-in-c/)
- [Oceano: 42 project walkthroughs (YouTube)](https://youtu.be/yTR00r8vBH8)

<br>

## AI Usage

AI was used as a support tool, not a code generator:

- **Planning:** the architecture and the five-stage pipeline before writing code.
- **Edge cases:** hunting quoting, heredoc, and signal corners against `bash` behaviour.
- **Documentation:** this README and the per-module docs under `src/`.

All code was written, understood, and is defensible by us.
