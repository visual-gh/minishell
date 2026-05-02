# 06 — Executor

Take a `t_cmd[]` and run it. Single command vs pipeline; builtins vs externals; redirections; heredocs; exit-status capture.

## Decision tree

```
n_cmds == 1 ?
├── yes → is argv[0] a builtin AND no pipe context?
│         ├── yes → run builtin in PARENT (so cd/export/exit affect us)
│         └── no  → fork+exec one child, wait
└── no  → build pipeline of n_cmds children
          parent waits all, captures last child's status
```

**Critical**: builtins in a **pipeline** (`echo hi | cat`) MUST run in a forked child, otherwise they corrupt the parent's stdin/stdout via dup2. Only standalone builtins run in the parent.

## Single command path

```c
int run_single(t_shell *sh, t_cmd *c)
{
    if (!c->argv || !c->argv[0])
        return apply_redirs_only(c);   // e.g., `> file`
    if (is_builtin(c->argv[0]))
        return run_builtin_in_parent(sh, c);
    return fork_exec(sh, c);
}
```

`apply_redirs_only`: bash opens/truncates the files even when there's no command. `> a` creates `a`. Reproduce that.

## Pipeline path

For `n` commands, you need `n - 1` pipes. Allocate them all up front:

```c
int pipes[n - 1][2];
for (i = 0; i < n - 1; i++) pipe(pipes[i]);
```

Then fork `n` children. Each child:

```
if not first:  dup2(pipes[i-1][0], STDIN_FILENO)
if not last:   dup2(pipes[i  ][1], STDOUT_FILENO)
close all pipe fds in this child (read+write of every pipe)
apply this cmd's redirs (they OVERRIDE pipe redirs)
if builtin: run, exit(status)
else:       resolve path, execve
```

**Parent**: closes ALL pipe fds (otherwise children block forever waiting for EOF), then `waitpid` each child. Capture `last_status` from the **last** child (not first):

```c
for (i = 0; i < n; i++)
{
    waitpid(pids[i], &st, 0);
    if (i == n - 1)
        sh->last_status = exit_code_from(st);
}
```

`exit_code_from`:
```c
if (WIFEXITED(st))   return WEXITSTATUS(st);
if (WIFSIGNALED(st)) return 128 + WTERMSIG(st);
return 1;
```

While waiting, parent should ignore SIGINT (set handler to SIG_IGN) and restore after — otherwise Ctrl-C kills the parent. See `08_Signals.md`.

## Redirections — applying them

Walk `c->redirs[]` in order. For each:

```c
case R_IN:
    fd = open(target, O_RDONLY);
    if (fd < 0) { perror(target); exit(1); }
    dup2(fd, 0); close(fd);

case R_OUT:
    fd = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { perror(target); exit(1); }
    dup2(fd, 1); close(fd);

case R_APPEND:
    fd = open(target, O_WRONLY | O_CREAT | O_APPEND, 0644);
    ...

case R_HEREDOC:
    fd = heredoc_fd(redir);     // see below
    dup2(fd, 0); close(fd);
```

Inside a child: open-fail prints to stderr and exits 1.
Inside the parent (no-fork edge case): same, but set `last_status` and don't exit.

## Heredoc

Two viable strategies. Pick **A** (simpler, defensible).

### A) Pipe in parent, child reads

Parent reads heredoc lines from stdin (via `readline("> ")`) until a line matches the delimiter. Write the body to an unnamed pipe, give the read end to the child as fd 0.

```c
int heredoc_to_pipe(const char *delim, int expand, t_shell *sh)
{
    int  fds[2];
    pipe(fds);
    while (1)
    {
        char *line = readline("> ");
        if (!line || !ft_strcmp(line, delim)) { free(line); break; }
        char *out = expand ? expand_word(line, sh) : ft_strdup(line);
        write(fds[1], out, ft_strlen(out));
        write(fds[1], "\n", 1);
        free(out); free(line);
    }
    close(fds[1]);
    return fds[0];
}
```

Do this **before** building the pipeline so any heredoc Ctrl-C aborts cleanly.

### B) Temp file

`/tmp/.minishell_heredoc_<pid>_<n>`, write body, reopen for reading, unlink early. More fd-leak surface — only choose if A doesn't fit.

### Heredoc + Ctrl-C

If the user hits Ctrl-C during heredoc input, abort: free buffers, set `last_status = 130`, skip executing the command. The signal handler sets `g_signal = SIGINT`; after each `readline("> ")` returns, check `g_signal`. (Also, readline returns NULL on Ctrl-D — treat as a polite EOF, print warning.)

## PATH resolution

```c
char *resolve_path(char *cmd, char **envp)
{
    if (ft_strchr(cmd, '/'))           // absolute or relative path
        return access(cmd, F_OK) == 0 ? ft_strdup(cmd) : NULL;
    char *path = getenv_value(envp, "PATH");
    if (!path) return NULL;
    char **dirs = ft_split(path, ':');
    for (int i = 0; dirs[i]; i++) {
        char *full = path_join(dirs[i], cmd);
        if (access(full, X_OK) == 0)   // X_OK matters
            return free_split(dirs), full;
        free(full);
    }
    free_split(dirs);
    return NULL;
}
```

Errors:
- `cmd` contains `/` and not found → `127` `"No such file or directory"`
- `cmd` contains `/` and not executable → `126` `"Permission denied"`
- bare `cmd`, no PATH, not in any PATH dir → `127` `"command not found"`
- found but `execve` fails (e.g., directory) → `126`

Match bash exactly:
```
minishell: <cmd>: command not found
minishell: <cmd>: No such file or directory
minishell: <cmd>: Permission denied
minishell: <cmd>: Is a directory
```

## fork+exec template

```c
pid_t pid = fork();
if (pid == 0)
{
    signals_default();          // restore default for child
    apply_redirs(c);            // may exit on failure
    if (is_builtin(c->argv[0]))
        exit(run_builtin(sh, c));
    char *path = resolve_path(c->argv[0], sh->envp);
    if (!path) { not_found(c->argv[0]); exit(127); }
    execve(path, c->argv, sh->envp);
    perror(path);
    exit(126);
}
// parent
```

## fd hygiene checklist

- [ ] Every `open` has a matching `close`
- [ ] Every pipe's both ends closed in parent before waiting
- [ ] dup2 followed by close of original
- [ ] No fd leaks across iterations (`valgrind --track-fds=yes`)
- [ ] Heredoc temp pipes/files cleaned up even on error path
