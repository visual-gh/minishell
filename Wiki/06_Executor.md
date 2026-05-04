# 06 — Executor

Take the cmd list (`shell->cmds`) and run it. Single command vs pipeline; builtins vs externals; redirections; exit-status capture. Heredoc bodies are already prepared by the heredoc stage — the executor just hooks the right fds.

## Decision tree

```
count cmds in shell->cmds
n == 1 ?
├── yes → is argv[0] a builtin AND no pipe context?
│         ├── yes → run builtin in PARENT (so cd/export/exit affect us)
│         └── no  → fork+exec one child, wait
└── no  → build pipeline of n children
          parent waits all, captures last child's status
```

**Critical**: builtins in a **pipeline** (`echo hi | cat`) MUST run in a forked child, otherwise they corrupt the parent's stdin/stdout via dup2. Only standalone builtins run in the parent.

## Single command path

```c
int run_single(t_shell *sh, t_cmd *c)
{
    if (!c->argv || !c->argv[0])
        return (apply_redirs_only(c));   /* e.g., `> file` */
    if (is_builtin(c->argv[0]))
        return (run_builtin_in_parent(sh, c));
    return (fork_exec(sh, c));
}
```

`apply_redirs_only`: bash opens/truncates the files even when there's no command. `> a` creates `a`. Reproduce that.

## Pipeline path

Walk the cmd list once to count it. Then allocate runtime arrays:

```c
int     n;
int     **pipes;     /* malloc'd: n-1 entries, each int[2] */
pid_t   *pids;       /* malloc'd: n entries */
```

VLAs (`int pipes[n - 1][2]`) are forbidden by the Norm — always heap-allocate.

```c
n = count_cmds(sh->cmds);
pipes = alloc_pipes(n - 1);     /* opens all pipes */
pids  = malloc(sizeof(pid_t) * n);
```

Then walk the cmd list a second time, forking one child per node:

```
i = 0
for cmd = sh->cmds; cmd; cmd = cmd->next, i++:
    pids[i] = fork()
    if pid == 0:
        if i > 0:     dup2(pipes[i-1][0], STDIN_FILENO)
        if cmd->next: dup2(pipes[i  ][1], STDOUT_FILENO)
        close all pipe fds in this child
        apply this cmd's redirs (they OVERRIDE pipe redirs)
        if builtin: run, exit(status)
        else:       resolve path, execve
```

(In real C-under-Norm: write that loop as a `while`, not a `for`.)

**Parent**: closes ALL pipe fds (otherwise children block forever waiting for EOF), then `waitpid` each child. Capture `last_status` from the **last** child:

```c
i = 0;
while (i < n)
{
    waitpid(pids[i], &st, 0);
    if (i == n - 1)
        sh->last_status = exit_code_from(st);
    i++;
}
```

`exit_code_from`:
```c
if (WIFEXITED(st))   return (WEXITSTATUS(st));
if (WIFSIGNALED(st)) return (128 + WTERMSIG(st));
return (1);
```

While waiting, parent should ignore SIGINT — call `signals_wait()` before the wait loop and `signals_prompt()` after. Otherwise Ctrl-C kills the parent. See `08_Signals.md`.

## Redirections — applying them

Walk `cmd->redirs` (linked list, head-to-tail). For each node:

```c
case REDIR_IN:
    fd = open(r->target, O_RDONLY);
    if (fd < 0) { perror(r->target); exit(1); }
    dup2(fd, 0); close(fd);

case REDIR_OUT:
    fd = open(r->target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { perror(r->target); exit(1); }
    dup2(fd, 1); close(fd);

case REDIR_APPEND:
    fd = open(r->target, O_WRONLY | O_CREAT | O_APPEND, 0644);
    ...

case REDIR_HEREDOC:
    fd = r->heredoc_fd;        /* already prepared by read_heredocs */
    dup2(fd, 0); close(fd);
```

For `REDIR_HEREDOC`, the parent already wrote the body to a pipe (or temp file) during the heredoc stage and stored the read end on the redir node (e.g., as a private field — extend `t_redir` if needed, or keep a parallel array; pick one, document it).

Inside a child: open-fail prints to stderr and exits 1.
Inside the parent (no-fork edge case): same, but set `last_status` and don't exit.

## Heredoc stage (separate from executor)

`read_heredocs(t_shell *shell)` runs **before** the executor builds the pipeline. For every `REDIR_HEREDOC` in every cmd:

### Strategy A — Pipe in parent, child reads (recommended)

Parent reads heredoc lines from stdin (via `readline("> ")`) until a line matches the delimiter. Write the body to an unnamed pipe; later, the executor's `apply_redirs` will dup the read end onto fd 0 in the child.

```c
int heredoc_to_pipe(const char *delim, int expand, t_shell *sh)
{
    int  fds[2];
    char *line;
    char *out;

    pipe(fds);
    signals_heredoc();
    while (1)
    {
        line = readline("> ");
        if (!line || !ft_strcmp(line, delim))
        {
            free(line);
            break;
        }
        out = expand ? expand_word(line, sh) : ft_strdup(line);
        write(fds[1], out, ft_strlen(out));
        write(fds[1], "\n", 1);
        free(out);
        free(line);
    }
    close(fds[1]);
    signals_prompt();
    return (fds[0]);
}
```

Do this **before** building the pipeline so any heredoc Ctrl-C aborts cleanly without leaving zombies.

### Strategy B — Temp file

`/tmp/.minishell_heredoc_<pid>_<n>`, write body, reopen for reading, unlink early. More fd-leak surface — only choose if A doesn't fit.

### Heredoc + Ctrl-C

If the user hits Ctrl-C during heredoc input, abort: free buffers, set `last_status = 130`, skip executing the command. The signal handler sets `g_signal = SIGINT` and closes stdin so `readline("> ")` returns NULL. After it returns, check `g_signal` and propagate.

## PATH resolution

```c
char *resolve_path(char *cmd, char **envp)
{
    char    *path;
    char    **dirs;
    char    *full;
    int     i;

    if (ft_strchr(cmd, '/'))
        return (access(cmd, F_OK) == 0 ? ft_strdup(cmd) : NULL);
    path = env_get(envp, "PATH");
    if (!path)
        return (NULL);
    dirs = ft_split(path, ':');
    i = 0;
    while (dirs[i])
    {
        full = path_join(dirs[i], cmd);
        if (access(full, X_OK) == 0)
            return (free_str_array(dirs), full);
        free(full);
        i++;
    }
    return (free_str_array(dirs), NULL);
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

Centralise these via `print_error(cmd, arg, msg)` from the header.

## fork+exec template

```c
pid_t   pid;
char    *path;

pid = fork();
if (pid == 0)
{
    signals_child();              /* restore default for child */
    apply_redirs(c->redirs);      /* may exit on failure */
    if (is_builtin(c->argv[0]))
        exit(run_builtin(c, sh));
    path = resolve_path(c->argv[0], sh->envp);
    if (!path)
    {
        print_error(c->argv[0], NULL, "command not found");
        exit(127);
    }
    execve(path, c->argv, sh->envp);
    perror(path);
    exit(126);
}
/* parent */
```

## fd hygiene checklist

- [ ] Every `open` has a matching `close`
- [ ] Every pipe's both ends closed in parent before waiting
- [ ] dup2 followed by close of original
- [ ] No fd leaks across iterations (`valgrind --track-fds=yes`)
- [ ] Heredoc pipes/files cleaned up even on error path
