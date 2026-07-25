# src/signals/: Signal Layer

Signals are how the OS asks the shell to react to things like Ctrl-C (`SIGINT`) or Ctrl-\\ (`SIGQUIT`).
This file owns the handler that runs on those signals and the small global flag they use to talk to the rest of the shell.

---

## Overview

```c
volatile sig_atomic_t  g_signal = 0;     // breadcrumb from handler -> main loop

void         signals_prompt(void);       // at the prompt: Ctrl-C redraws, Ctrl-\ ignored
void         signals_child(void);        // a running command: default behaviour
void         signals_wait(void);         // shell waiting on a child: ignore both
void         signals_heredoc(void);      // reading a heredoc: Ctrl-C aborts
```

The shell wears a different "signal outfit" depending on what it's doing:
waiting at the prompt, running a child, or reading a heredoc. Each of the four
functions installs the right handlers for that moment, and callers swap between
them. `shell_init` calls `signals_prompt()` once at startup; from then on, every
Ctrl-C at the prompt runs `sigint_prompt`, which redraws the line and leaves
`g_signal = SIGINT` behind for `shell_loop` to find.

---

## `g_signal`: the breadcrumb

When a signal fires, the kernel runs your handler in a special restricted context. You **cannot** safely touch `t_shell *` from inside it: most libc functions are off-limits, and a partial write to the struct could leave it in a broken state.

So the handler does the bare minimum (redraw the prompt) and leaves a one-byte note:

```c
g_signal = sig;
```

Back in `shell_loop`, after `readline()` returns, we check the note:

```c
if (g_signal == SIGINT)
{
    shell->last_status = 130;   // bash sets $? to 130 after Ctrl-C
    g_signal = 0;               // clear it for next iteration
}
```

That gives us the correct `$?` after a Ctrl-C without doing anything dangerous in signal context.

**Why `volatile sig_atomic_t`?**
- `volatile` tells the compiler the value can change between any two instructions (the handler can fire any time), so it must reload from memory instead of caching in a register.
- `sig_atomic_t` is the only integer type the C standard guarantees can be read and written in a single uninterruptible instruction, so we never see a half-updated value.

Together: a race-free flag with no locks. The Norm allows exactly one global in minishell, and this is the canonical use of it.

---

## `sigint_prompt`: the handler

```c
static void  sigint_prompt(int sig)
{
    g_signal = sig;
    write(1, "\n", 1);
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}
```

Four things, in order:

1. Drop the breadcrumb.
2. `write(1, "\n", 1)`: move the cursor down. We use `write` because it's signal-safe (`printf` is not).
3. The three `rl_*` calls are how you tell GNU readline "scrap whatever the user was typing and redraw a fresh empty prompt":
   - `rl_on_new_line()`: tell readline the cursor is now on a new line.
   - `rl_replace_line("", 0)`: erase the in-progress input buffer.
   - `rl_redisplay()`: repaint the prompt.

Static because only `sigaction`/`signal` calls it through the kernel, never us directly.

---

## `signals_prompt`: installing the handler

```c
void  signals_prompt(void)
{
    signal(SIGINT, sigint_prompt);
    signal(SIGQUIT, SIG_IGN);
}
```

`signal(num, handler)` is the simple form: "for signal `num`, run `handler`." Two arguments, no struct.

The two non-function values you'll see as a handler:
- `SIG_IGN`: ignore the signal entirely (what we use for `SIGQUIT`, since bash doesn't react to Ctrl-\\ at the prompt).
- `SIG_DFL`: restore the kernel's default action (terminate, core dump, etc).

On Linux/glibc, `signal()` is implemented on top of the more powerful `sigaction()` with `SA_RESTART` already set, so `readline`'s blocked `read()` is automatically resumed after the handler returns. Exactly what we need.

> If you ever need more control, like blocking other signals while the handler runs, `sigaction()` is the richer API. Mixing the two is fine; each call replaces whatever was installed before.

---

## The other three modes

Each is installed at the right moment and swapped back to `signals_prompt`
afterward.

**`signals_child`** is set in a forked child ([`run_child`](../executor/README.md))
just before it becomes the target program. It restores the **default** behaviour
so a running command reacts to Ctrl-C / Ctrl-\\ exactly as it would under bash:

```c
void signals_child(void)
{
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
}
```

**`signals_wait`** is set in the parent while it `waitpid`s on a child. Both
signals are ignored, so pressing Ctrl-C kills the *child* (which has `SIG_DFL`)
without disturbing the shell:

```c
void signals_wait(void)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}
```

The pairing is the whole trick: **child default + parent ignore** means a
foreground program can be interrupted while the shell survives. After the wait,
the caller restores `signals_prompt`.

**`signals_heredoc`** is set while reading a [heredoc](../heredoc/README.md) body.
It lives in its own file (`signals_heredoc.c`) because its handler does something
unusual: it closes stdin to break `readline` out of its blocking read.

```c
static void sigint_heredoc(int sig)
{
    g_signal = sig;
    close(STDIN_FILENO);   // makes the blocked readline("> ") return NULL
}
```

The [heredoc README](../heredoc/README.md) walks through the full abort sequence.

---

## Lifecycle

```
shell_init()
    └─> signals_prompt()           handlers installed

shell_loop()
    │
    ├── readline() blocks
    │       ▲
    │       │  Ctrl-C  →  sigint_prompt() : g_signal = SIGINT, redraw
    │       │
    │   readline returns
    │
    └── if (g_signal == SIGINT)
            shell->last_status = 130
            g_signal = 0
```
