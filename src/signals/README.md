# src/signals/: Signal Layer

Signals are how the OS asks the shell to react to things like Ctrl-C (`SIGINT`) or Ctrl-\\ (`SIGQUIT`).
This file owns the handler that runs on those signals and the small global flag they use to talk to the rest of the shell.

---

## Overview

```c
volatile sig_atomic_t  g_signal = 0;     // breadcrumb from handler -> main loop

static void  sigint_prompt(int sig);     // runs on Ctrl-C at the prompt
void         signals_prompt(void);       // installs the handlers above

void         signals_child(void);        // stub, phase 5
void         signals_wait(void);         // stub, phase 5
void         signals_heredoc(void);      // stub, phase 3
```

`shell_init` calls `signals_prompt()` once at startup. From then on, every Ctrl-C the user presses at the prompt runs `sigint_prompt`, which redraws the line and leaves `g_signal = SIGINT` behind for `shell_loop` to find.

---

## `g_signal`: the breadcrumb

When a signal fires, the kernel runs your handler in a special restricted context. You **cannot** safely touch `t_shell *` from inside it — most libc functions are off-limits, and a partial write to the struct could leave it in a broken state.

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
- `sig_atomic_t` is the only integer type the C standard guarantees can be read and written in a single uninterruptible instruction — so we never see a half-updated value.

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
2. `write(1, "\n", 1)` — move the cursor down. We use `write` because it's signal-safe (`printf` is not).
3. The three `rl_*` calls are how you tell GNU readline "scrap whatever the user was typing and redraw a fresh empty prompt":
   - `rl_on_new_line()`: tell readline the cursor is now on a new line.
   - `rl_replace_line("", 0)`: erase the in-progress input buffer.
   - `rl_redisplay()`: repaint the prompt.

Static because only `sigaction`/`signal` calls it through the kernel — never us directly.

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
- `SIG_IGN` — ignore the signal entirely (what we use for `SIGQUIT`, since bash doesn't react to Ctrl-\\ at the prompt).
- `SIG_DFL` — restore the kernel's default action (terminate, core dump, etc).

On Linux/glibc, `signal()` is implemented on top of the more powerful `sigaction()` with `SA_RESTART` already set, so `readline`'s blocked `read()` is automatically resumed after the handler returns. Exactly what we need.

> If a later phase (heredoc, child) needs more control — like blocking other signals while the handler runs — it can use `sigaction()` instead. Mixing the two APIs is fine; each call replaces whatever was installed before.

---

## Stubs

```c
void  signals_child(void)   { }   // phase 5 — child before execve: SIG_DFL on both
void  signals_wait(void)    { }   // phase 5 — parent waiting on a pipeline: SIG_IGN on both
void  signals_heredoc(void) { }   // phase 3 — heredoc read: Ctrl-C aborts, no redraw
```

Empty for now — declared so the public API stays stable. Phase 3 and phase 5 will fill them in.

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
