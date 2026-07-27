# src/executor/: Running the commands

The executor is the last stage and the heart of the project. It takes the fully
parsed, expanded [command list](../../includes/README.md) and actually runs it:
forking processes, wiring pipes and redirections, resolving executables, and
collecting the exit status that becomes `$?`.

If you understand `fork`, `pipe`, `dup2`, `execve`, and `waitpid`, you understand
this directory. If you don't yet, this README teaches them in the order the code
uses them.

---

## The four system calls, in one paragraph

`fork()` splits the current process into two identical copies (parent and child)
that share the same open file descriptors. `execve(path, argv, envp)` **replaces**
the calling process with a new program; it only returns if it failed. `pipe(fd)`
makes a one-way channel: bytes written to `fd[1]` come out of `fd[0]`.
`dup2(old, new)` points `new` at whatever `old` refers to, so
`dup2(fd, STDIN_FILENO)` means "from now on, reading stdin reads from `fd`."
`waitpid` blocks until a child finishes and reports how it died. That's the
whole toolkit.

---

## Top-level dispatch

```c
int execute(t_shell *shell)
{
    if (count_cmds(shell->cmds) == 1)
        return (run_single(shell, shell->cmds));   // one command
    return (run_pipeline(shell));                    // two or more
}
```

One command and a pipeline are handled separately because a single command has a
crucial special case: **built-ins that must run in the shell itself.**

---

## One command: `run_single`

```c
int run_single(t_shell *shell, t_cmd *cmd)
{
    if (!cmd->argv || !cmd->argv[0])
        return (run_with_redirs(shell, cmd));   // redirs only, e.g.  > file
    if (is_builtin(cmd->argv[0]))
        return (run_with_redirs(shell, cmd));   // builtin in THIS process
    return (fork_exec(shell, cmd));             // external program in a child
}
```

### Why a lone built-in runs in the parent

This is *the* classic minishell defense question. Consider:

```
cd /tmp
```

`cd` changes the working directory. If we ran it in a forked child, the child's
directory would change and then the child would exit; the shell itself would
never move. Same for `export`, `unset`, `exit`: they mutate shell state, so they
**must** run in the shell's own process, not a child. That's what
`run_with_redirs` does, with no fork.

The catch: redirections like `pwd > out.txt` still have to apply, but only for
the duration of the builtin, without wrecking the shell's real stdin/stdout. So
we save them, apply the redirs, run the builtin, and restore:

```c
saved_in = dup(STDIN_FILENO);
saved_out = dup(STDOUT_FILENO);
if (apply_redirs(cmd->redirs) < 0)
    status = 1;
else if (cmd->argv)
    status = run_builtin(cmd, shell);
else
    status = 0;                    // redir-only line: just open/create the file
dup2(saved_in, STDIN_FILENO);      // put stdout/stdin back
dup2(saved_out, STDOUT_FILENO);
close(saved_in);
close(saved_out);
```

(In a *pipeline*, builtins run in children instead (see below), which is why
`export X=1 | cat` does **not** set `X` in the shell. Bash behaves the same way.)

### An external program: `fork_exec`

```c
pid = fork();
if (pid == 0)
    run_child(shell, cmd);   // child: never returns (execve or exit)
signals_wait();              // parent ignores Ctrl-C while the child runs
waitpid(pid, &wstatus, 0);
signals_prompt();            // restore the prompt handler
return (exit_code_from(wstatus));
```

While the child runs, the parent switches to `signals_wait` (ignore Ctrl-C /
Ctrl-\\) so that pressing Ctrl-C kills the *child*, not the shell. See
[signals/](../signals/README.md).

---

## Inside the child: `run_child`

Every forked process (single external command or pipeline stage) ends up here.
It's the point of no return: it either `execve`s into the target program or
`exit`s with a status.

```c
void run_child(t_shell *shell, t_cmd *cmd)
{
    signals_child();                        // restore default Ctrl-C / Ctrl-\ 
    if (apply_redirs(cmd->redirs) < 0)
        exit(1);
    if (!cmd->argv || !cmd->argv[0])
        exit(0);                            // a redir-only pipeline stage
    if (is_builtin(cmd->argv[0]))
        exit(run_builtin(cmd, shell));      // builtin in a pipeline: OK to fork
    path = resolve_path(cmd->argv[0], shell->envp);
    if (!path)
    {
        print_error(cmd->argv[0], NULL, "command not found");
        exit(127);
    }
    envp = exported_envp(shell->envp);       // drops valueless export marks
    execve(path, cmd->argv, envp);
    perror(path);                            // only reached if execve failed
    exit(126);
}
```

The exit codes are bash's: **127** = command not found, **126** = found but not
executable (permission denied, or it's a directory). The
`if (!cmd->argv || !cmd->argv[0]) exit(0)` line guards the case of a pipeline
stage that is *only* a redirection, like `< Makefile | cat`; without it, that
first stage would dereference a NULL `argv` and crash.

---

## A pipeline: `run_pipeline`

For `a | b | c`, we need two pipes: one connecting `a→b`, one connecting `b→c`.
In general, `n` commands need `n-1` pipes.

```c
total = count_cmds(shell->cmds);
pipes = alloc_pipes(total - 1);          // create all pipes up front
pids = spawn_children(shell, pipes, total);
close_pipes(pipes);                       // PARENT closes every pipe end...
status = wait_all(pids, total);           // ...then waits for all children
```

### Wiring one stage: `child_body`

Each child connects to its neighbours and then closes **every** pipe fd it holds:

```c
static void child_body(t_shell *shell, t_cmd *cmd, int **pipes, int i)
{
    if (i > 0)
        dup2(pipes[i - 1][0], STDIN_FILENO);   // read from the previous pipe
    if (cmd->next)
        dup2(pipes[i][1], STDOUT_FILENO);      // write into the next pipe
    close_pipes(pipes);                        // drop all raw fds, keep only dups
    run_child(shell, cmd);
}
```

Picture `a | b | c`:

```
          pipe0            pipe1
        ┌────────┐       ┌────────┐
  a ───►│ [1] [0]│──► b ─│[1]  [0]│──► c
 stdout └────────┘ stdin └────────┘ stdin
                    stdout
```

- `a` (i=0): no previous pipe; stdout → `pipe0[1]`.
- `b` (i=1): stdin ← `pipe0[0]`; stdout → `pipe1[1]`.
- `c` (i=2): stdin ← `pipe1[0]`; no next pipe.

### Why everyone closes every pipe

This is the other classic defense question and the #1 source of pipeline bugs. A
pipe's reader only sees EOF when **all** write ends are closed. The parent forked
before closing anything, so every child inherits copies of every pipe fd. If any
process keeps a write end open, the reader downstream blocks forever waiting for
input that will never come. So: each child `dup2`s the ends it needs and
immediately `close_pipes` (dropping every raw fd), and the parent closes all of
them too right after forking. Only the `dup2`'d copies on stdin/stdout survive.

### Collecting the status

```c
while (i < total)
{
    waitpid(pids[i], &wstatus, 0);
    if (i == total - 1)                 // the LAST command decides $?
        status = exit_code_from(wstatus);
    i++;
}
```

`$?` for a pipeline is the exit status of the **last** command, just like bash.
We still `waitpid` every child so none are left as zombies.

---

## Turning `wstatus` into an exit code

`waitpid` fills a packed status word; `exit_code_from` unpacks it:

```c
int exit_code_from(int wstatus)
{
    if (WIFEXITED(wstatus))
        return (WEXITSTATUS(wstatus));       // normal exit → its code
    if (WIFSIGNALED(wstatus))
    {
        report_quit(wstatus);                // "Quit (core dumped)" on Ctrl-\
        return (128 + WTERMSIG(wstatus));    // killed by signal → 128+n
    }
    return (1);
}
```

`128 + n` is why Ctrl-C'ing `cat` gives `$? == 130` (SIGINT is 2), and Ctrl-\\
gives 131 (SIGQUIT is 3). `report_quit` prints bash's message for that second
case, with `(core dumped)` only when `WCOREDUMP` says a core was written. Every
wait in the shell funnels through here, so both callers got it for free.

---

## Applying redirections: `redirs.c`

`apply_redirs` walks a command's redirection list and rewires the standard fds:

```c
if (redirs->type == REDIR_IN)
    ret = open_in(redirs->target);                                  //  <
else if (redirs->type == REDIR_OUT)
    ret = open_out(redirs->target, O_WRONLY | O_CREAT | O_TRUNC);   //  >
else if (redirs->type == REDIR_APPEND)
    ret = open_out(redirs->target, O_WRONLY | O_CREAT | O_APPEND);  //  >>
else
    ret = open_heredoc(redirs);                                     //  <<
```

`open_in`/`open_out` open the file, `dup2` it onto stdin/stdout, and close the
original fd. `open_heredoc` is trivial: the [heredoc stage](../heredoc/README.md)
already read the body into a pipe, so this just `dup2`s `heredoc_fd` onto stdin.
When multiple redirections target the same stream, the last one wins, because
each `dup2` overwrites the previous, exactly bash's `> a > b` behaviour.

On failure, the error is prefixed with `minishell: ` to match bash:

```c
if (fd < 0)
    return (ft_putstr_fd("minishell: ", 2), perror(target), -1);
```

---

## Finding the program: `resolve_path`

```c
if (ft_strchr(cmd, '/'))            // already a path? use it as-is
    return (resolve_slash(cmd));
path = env_get(envp, "PATH");       // otherwise search $PATH, left to right
dirs = ft_split(path, ':');
while (dirs[i])
{
    full = path_join(dirs[i], cmd);           // "dir" + "/" + "ls"
    if (full && access(full, X_OK) == 0)      // first executable match wins
        return (free_str_array(dirs), full);
    ...
}
```

If the command contains a `/` it's treated as a literal path (relative or
absolute). Otherwise each `PATH` directory is tried in order and the first one
where `access(..., X_OK)` succeeds is used, which is why unsetting `PATH` makes
bare commands fail, and why directory order matters.

---

## The files

| File | What's in it |
|---|---|
| `executor.c` | `execute` (dispatch), `exit_code_from`, `is_builtin` |
| `single.c` | `run_single`, `run_with_redirs` (builtin in parent), `fork_exec` |
| `run_child.c` | `run_child`: the child's execve-or-exit path |
| `pipeline.c` | `run_pipeline`, `spawn_children`, `child_body`, `wait_all` |
| `pipeline_utils.c` | `count_cmds`, `alloc_pipes`, `close_pipes` |
| `redirs.c` | `apply_redirs` and the `open_*` helpers |
| `resolve_path.c` | `resolve_path`: `/`-path or `PATH` search |
| `run_builtin.c` | `run_builtin`: dispatch a name to its [builtin](../builtins/README.md) |

---

## Memory & fds

The executor allocates little and cleans up eagerly: `alloc_pipes` builds all the
pipe fds and `close_pipes` frees and closes every one (even on error, it's the
single cleanup path). `resolve_path` frees the split `PATH` array before
returning. `spawn_children` frees the `pids` array after waiting. The
[command list](../../includes/README.md) itself is freed by the caller
(`process_line`) after `execute` returns; see the [free family](../utils/README.md).

**Built-ins** dispatched from here live in [builtins/](../builtins/README.md);
the environment they read and write is [env/](../env/README.md).
