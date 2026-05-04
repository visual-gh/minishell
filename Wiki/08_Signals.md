# 08 — Signals

The most fragile part of the project. Get the model right first, then code 30 lines.

## The one global rule

> "Use at most one global variable to indicate a received signal. ... must only store the signal number and must not provide any additional information or access to data."

```c
// signals.c
volatile sig_atomic_t g_signal = 0;
```

That's it. **No struct, no array, no pointer.** Just an atomic int. Inside the handler, ONLY:

```c
void sigint_handler(int sig)
{
    g_signal = sig;
    // (special readline dance — see below)
}
```

Everywhere else (after readline returns, after waitpid returns) you read `g_signal`, react, and reset it to 0.

## Four install points the shell goes through

The handlers must be **swapped** depending on the context. The header exposes:

```c
void    signals_prompt(void);    /* mode 1: REPL waiting on readline */
void    signals_wait(void);      /* mode 2a: parent waiting on a child */
void    signals_child(void);     /* mode 2b: child process before execve */
void    signals_heredoc(void);   /* mode 3: reading heredoc body */
```

Modes 2a and 2b are two sides of the same "child running" situation — the parent ignores SIGINT (so Ctrl-C goes only to the child), and the child restores defaults so it actually dies on Ctrl-C.

### Mode 1: Interactive prompt (waiting on `readline`)

| Signal | Behavior |
|---|---|
| Ctrl-C (SIGINT) | print newline, redisplay an empty prompt, set `last_status = 130` |
| Ctrl-\\ (SIGQUIT) | ignored |
| Ctrl-D (EOF) | readline returns NULL → exit shell |

Handler for SIGINT in this mode:
```c
void sigint_prompt(int sig)
{
    g_signal = sig;
    write(1, "\n", 1);
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}
```

`signal(SIGQUIT, SIG_IGN);`

### Mode 2: Foreground child running (parent in `waitpid`)

| Signal | Behavior |
|---|---|
| Ctrl-C | parent ignores; kernel delivers to child (foreground process group). After child dies, status = 130. Print newline ourselves so prompt is on a fresh line. |
| Ctrl-\\ | same idea — child dies with `Quit (core dumped)`; bash prints that. Status = 131. |

So **before fork**: `signals_wait()` → `SIG_IGN` for SIGINT and SIGQUIT in the parent.
**In child, before exec**: `signals_child()` → restore defaults (`SIG_DFL`).
**After waitpid**: `signals_prompt()` re-installs the prompt-mode handlers.

If `WTERMSIG(status) == SIGINT` → write `"\n"` to keep prompt clean. If `WTERMSIG(status) == SIGQUIT` → write `"Quit (core dumped)\n"`.

### Mode 3: Heredoc input (`readline("> ")`)

| Signal | Behavior |
|---|---|
| Ctrl-C | abort heredoc; `last_status = 130`; skip command |
| Ctrl-D | EOF → warn `minishell: warning: here-document delimited by end-of-file` and continue with what was collected |

In heredoc mode, install a SIGINT handler that:
```c
void sigint_heredoc(int sig)
{
    g_signal = sig;
    close(STDIN_FILENO);   // forces readline to return NULL
}
```
After readline returns NULL, check `g_signal == SIGINT` → discard heredoc, set status, propagate.

(Some folks use `rl_done = 1` instead of closing stdin; both work.)

## Install/swap helpers

```c
void    signals_prompt(void)
{
    struct sigaction    sa;

    ft_memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_prompt;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    signal(SIGQUIT, SIG_IGN);
}

void    signals_wait(void)            /* parent, child running */
{
    signal(SIGINT,  SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}

void    signals_child(void)           /* in child before exec */
{
    signal(SIGINT,  SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
}

void    signals_heredoc(void)
{
    signal(SIGINT,  sigint_heredoc);
    signal(SIGQUIT, SIG_IGN);
}
```

## REPL skeleton

```c
while (1)
{
    signals_prompt();
    line = readline("minishell$ ");
    if (!line)            /* Ctrl-D */
    {
        ft_putendl_fd("exit", 1);
        break ;
    }
    if (g_signal == SIGINT)
    {
        sh.last_status = 130;
        g_signal = 0;
    }
    if (*line)
        add_history(line);
    process_line(&sh, line);
    free(line);
}
```

## Why these choices

- **`SA_RESTART`** prevents `read`/`waitpid` from EINTR-aborting on signal.
- **No global struct** — graders look for it. The norm rule says global must be `static`/`const`. But the project explicitly carves out the signal global. Make it `volatile sig_atomic_t` and document why.
- **Don't print from the handler** beyond a single `write` — async-signal-safe rule.
- **Don't access `t_shell` from the handler** — that's exactly what the subject forbids.

## Test matrix

| State | Action | Expected |
|---|---|---|
| empty prompt | Ctrl-C | new prompt on next line, `$?=130` |
| empty prompt | Ctrl-D | shell exits, prints `exit` |
| empty prompt | Ctrl-\\ | nothing |
| `cat` running | Ctrl-C | cat dies, prompt back, `$?=130` |
| `cat` running | Ctrl-\\ | `Quit (core dumped)`, prompt back, `$?=131` |
| `sleep 5 \| cat` | Ctrl-C | both die, $?=130 |
| `cat << EOF` | Ctrl-C | abort, no exec, $?=130 |
| `cat << EOF` | Ctrl-D | warning, partial body fed to cat |
