# src/heredoc/: Reading `<<` bodies

A heredoc lets you feed several lines of input to a command inline:

```
cat << EOF
hello $USER
EOF
```

`<< EOF` means *"read lines until one is exactly `EOF`, and feed everything
before it to `cat`'s stdin."* This stage reads that body and stashes it where the
[executor](../executor/README.md) can hand it to the command.

It's small but it's where people lose points in defense, because of three subtle
decisions: **when** it runs, **how** the body reaches the command, and what
happens on **Ctrl-C**.

---

## When: before anything executes

Look at `process_line`'s guard again:

```c
return (shell->cmds && read_heredocs(shell) == 0 && expand_cmds(shell) == 0);
```

`read_heredocs` runs **first**, before expansion, before forking. That's not
arbitrary. A heredoc is interactive input: the shell has to prompt you with `>`
and collect the body *now*, while it still owns the terminal, not after it has
forked children fighting over stdin. So the whole command list is scanned for
heredocs and all their bodies are read up front.

---

## How: the body goes into a pipe

For each heredoc, `read_heredoc_body` creates a pipe, reads lines until the
delimiter, writes them into the write end, and stores the **read end** on the
redirection:

```c
if (pipe(pipefd) < 0)
    return (-1);
line = readline("> ");
while (line && !g_signal && ft_strncmp(line, redir->target, SIZE_MAX) != 0)
{
    write_heredoc_line(pipefd[1], line, redir, shell);
    free(line);
    line = readline("> ");
}
free(line);
close(pipefd[1]);                 // done writing: reader will see EOF
redir->heredoc_fd = pipefd[0];    // executor will dup2 this onto stdin
```

Later, when the executor applies redirections, a `REDIR_HEREDOC` is just a
`dup2(redir->heredoc_fd, STDIN_FILENO)`: the command reads the body as if it came
from a file. A pipe is perfect here: it's an in-memory FIFO, no temp file to
create or clean up. (`ft_strncmp(line, target, SIZE_MAX)` is the project's
`strcmp`: libft has no `ft_strcmp`, and `SIZE_MAX` makes `strncmp` compare the
whole strings.)

Because heredocs are read before `expand_cmds`, and their bodies are expanded
here, the delimiter comparison uses the raw line, exactly what bash does.

---

## Expansion inside the body, and the quote trap

Whether the body expands `$VAR` depends on how the **delimiter** was written:

```
cat << EOF        →  body expands $USER   (unquoted delimiter)
cat << "EOF"      →  body is literal      (quoted delimiter)
```

That single bit is `redir->quoted`, set back in the [parser](../parser/README.md)
by `strip_quotes` when it processed the delimiter token. So:

```c
if (!redir->quoted)
    exp = expand_heredoc_line(line, shell);   // resolve $VAR
else
    exp = ft_strdup(line);                     // verbatim
```

Now the trap. `expand_heredoc_line` looks almost identical to the
[expander's](../expander/README.md) `expand_word` (both walk the line handling
`$`), so it's tempting to delete it and just call `expand_word`. **That would be
a bug.** Inside a heredoc body, bash removes *nothing*: quotes are literal.

```
cat << EOF
it's "quoted" $USER
EOF
→  it's "quoted" visual        # quotes stay, only $USER expands
```

`expand_word` strips quotes; a heredoc body must keep them. So this stage keeps
its own line-expander that reuses `expand_var` for the `$` part but never touches
quotes:

```c
if (line[i] == '$')
    tmp = expand_var(line, &i, shell);
else
    tmp = scan_heredoc_literal(line, &i);   // copy the run up to the next $, quotes and all
```

The two functions differ *on purpose*. (`scan_heredoc_literal` grabs whole runs
between `$`s for the same O(n²) reason explained in the
[expander README](../expander/README.md).)

---

## Ctrl-C during a heredoc

While reading a body, `Ctrl-C` should abort the whole heredoc and return to a
fresh prompt, not just cancel one line. That needs a different signal handler
than the normal prompt, so `read_heredocs` swaps it in and out:

```c
saved_stdin = dup(STDIN_FILENO);   // remember the real stdin
signals_heredoc();                 // install the heredoc handler
... read all heredoc bodies ...
dup2(saved_stdin, STDIN_FILENO);   // restore stdin
close(saved_stdin);
signals_prompt();                  // restore the normal handler
if (g_signal)                      // interrupted?
{
    shell->last_status = 130;
    g_signal = 0;
    ret = -1;                      // → process_line skips execution
}
```

The heredoc handler ([signals/](../signals/README.md)) does something specific:

```c
static void sigint_heredoc(int sig)
{
    g_signal = sig;
    close(STDIN_FILENO);   // force the blocked readline("> ") to return NULL
}
```

`readline` is blocked waiting for input; closing stdin under it makes it return
`NULL`, which breaks the read loop (`while (line && !g_signal && ...)`). Then
`read_heredocs` sees `g_signal`, sets `$?` to 130, and returns `-1` so the
command never runs. `saved_stdin` is how we put the terminal back afterward.

> Defense favourite: *"what happens if you Ctrl-C in the middle of a heredoc?"*
> Walk them through this: custom handler → close stdin → readline returns NULL →
> loop exits → status 130 → line discarded → stdin restored.

---

## The files

| File | What's in it |
|---|---|
| `heredoc.c` | `read_heredocs` (orchestration, signal swap), `read_heredoc_body` (the read loop), `write_heredoc_line` |
| `heredoc_expand.c` | `expand_heredoc_line`: `$` expansion **without** quote removal |
| (handler) | `signals_heredoc` lives in [signals/](../signals/README.md) |

---

## Memory & fds

Each heredoc holds one pipe read-fd in `redir->heredoc_fd`. The write end is
closed as soon as the body is written. The read end is closed by the executor
after it `dup2`s it onto stdin. On a Ctrl-C abort, the read end is closed right
away (`close(pipefd[0])`) so nothing leaks. Every `line` from `readline` is
freed each iteration.

**Next:** the [expander](../expander/README.md) runs on the (now heredoc-free)
command list, then the [executor](../executor/README.md) wires
`heredoc_fd` onto stdin.
