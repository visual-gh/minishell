# 01 — Overview

## What is a shell?

A shell is a **REPL** (Read-Eval-Print Loop) sitting between the user and the kernel. It reads a line of text, parses it into commands, executes them via `fork`+`execve`, and prints output. Bash, zsh, sh — all variants of the same idea.

Minishell = a stripped-down bash. You replicate a useful subset of its behavior.

## What you must support (mandatory)

| Feature | Example |
|---|---|
| Prompt | `minishell$ ` shown when waiting |
| History | `↑` recalls previous lines (readline gives this) |
| PATH lookup | `ls` finds `/usr/bin/ls` |
| Absolute/relative paths | `./a.out`, `/bin/ls` |
| Single quotes | `'$USER'` → literal `$USER` |
| Double quotes | `"$USER"` → expanded, but spaces preserved |
| Redirections | `<`, `>`, `<<`, `>>` |
| Pipes | `cmd1 \| cmd2 \| cmd3` |
| Env expansion | `$HOME`, `$?` |
| Signals | Ctrl-C, Ctrl-D, Ctrl-\\ |
| Builtins | `echo -n`, `cd`, `pwd`, `export`, `unset`, `env`, `exit` |

## What you must NOT support

- `;` (statement separator)
- `\` line continuation / escaping
- `&&`, `||`, `()` — bonus only
- `*` wildcards — bonus only
- Subshells, command substitution `$(...)` / backticks
- Tab completion (readline sometimes does it; don't rely on it)
- Job control (`&`, `bg`, `fg`)
- Arithmetic `$((...))`

## Mental model in one diagram

```
   raw line
      │
      ▼
   ┌─────────┐
   │  LEXER  │  → list of tokens (WORD, PIPE, REDIR_IN, ...)
   └─────────┘     each token tracks its quote-state
      │
      ▼
   ┌─────────┐
   │ PARSER  │  → flat command table: t_cmd[]
   └─────────┘     each cmd has: argv (raw), redirs[], plus syntax check
      │
      ▼
   ┌──────────┐
   │ EXPANDER │  → in-place: replace $VAR / $?, drop quote chars,
   └──────────┘     respect quote rules; split nothing for mandatory
      │
      ▼
   ┌──────────┐
   │ EXECUTOR │  → fork pipeline, dup2 redirs, execve, waitpid
   └──────────┘     captures last exit status into shell state
      │
      ▼
   loop back
```

## Scope philosophy (Visual + duo)

- Mandatory only. No bonus, no over-engineering.
- One global variable, used **only** to store the last received signal number.
- Flat command table — no AST. AST is overkill without `&&`/`||`/subshells.
- Centralized error path: every malloc-checked, every fd closed, every child waited.
- Test against bash literally: same input → same output → same exit code.

## Time budget (12 weeks, ~140h each)

| Phase | Weeks | Output |
|---|---|---|
| Setup, lexer, parser skeleton | 1–3 | tokens + flat cmd table, syntax errors |
| Expander, env, signals, readline | 4–5 | interactive prompt, $VAR works |
| Executor: single cmd, builtins | 6–7 | echo/pwd/cd/env work, exit codes |
| Pipes + redirections | 8–9 | full pipelines, `<<` heredoc |
| Edge cases, valgrind, norm | 10–11 | leak-free, norm-clean |
| Defense prep, README, buffer | 12 | rehearse, peer-review |
