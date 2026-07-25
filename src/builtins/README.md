# src/builtins/: The seven built-ins

A built-in is a command the shell runs **itself** instead of `execve`ing an
external program. The subject requires seven: `echo`, `cd`, `pwd`, `export`,
`unset`, `env`, `exit`. Each lives in its own file and returns an `int` exit
status.

Why are these built into the shell at all? Because most of them change the
shell's own state: the current directory, the environment, whether it keeps
running. An external `/bin/cd` couldn't work: it would run in a child and its
directory change would vanish when the child exits. The
[executor](../executor/README.md) explains *where* they run (parent for a lone
builtin, child inside a pipeline); this README explains *what each one does*.

Dispatch is a simple name match in `run_builtin` ([executor](../executor/README.md)):

```c
if (!ft_strncmp(name, "cd", SIZE_MAX))
    return (ft_cd(cmd, shell));
if (!ft_strncmp(name, "echo", SIZE_MAX))
    return (ft_echo(cmd));
...
```

---

## `echo` (with `-n`)

Print the arguments separated by single spaces, followed by a newline unless
`-n` is given. The only twist is recognising the flag, which accepts `-n`, `-nn`,
`-nnnn`… but nothing else (`-na` is a normal argument):

```c
static int is_echo_flag(char *str)
{
    if (str[0] != '-' || str[1] != 'n')
        return (0);
    i = 1;
    while (str[i] == 'n')
        i++;
    return (str[i] == '\0');      // only 'n's after the '-'
}
```

Leading flags are consumed to turn the trailing newline off; the rest are
printed. `echo -n -n hi` prints `hi` with no newline.

---

## `cd` (relative or absolute path)

Move the working directory, and keep `PWD` / `OLDPWD` in the environment in sync
so other tools see the right values:

```c
if (cmd->argv[1] && cmd->argv[2])                    // cd a b  → error
    return (print_error("cd", NULL, "too many arguments"), 1);
target = get_target(cmd, shell);                     // argv[1], or $HOME if none
if (getcwd(old_cwd, ...) == NULL || chdir(target) == -1)
    return (print_error("cd", target, strerror(errno)), 1);
env_set(&shell->envp, "OLDPWD", old_cwd);
if (getcwd(new_cwd, ...) != NULL)
    env_set(&shell->envp, "PWD", new_cwd);
```

- `cd` with no argument goes to `$HOME` (and errors if `HOME` isn't set).
- A failed `chdir` reports the target and the real reason via `strerror(errno)`,
  e.g. `minishell: cd: /nope: No such file or directory`, matching bash.
- `.` and `..` work for free, because `chdir` handles them.

---

## `pwd` (no options)

The whole builtin:

```c
if (getcwd(cwd, sizeof(cwd)) == NULL)
    return (perror("pwd"), 1);
ft_putendl_fd(cwd, STDOUT_FILENO);
return (0);
```

`getcwd` asks the kernel for the current directory; we print it. No reliance on
the `PWD` variable, so it stays correct even if someone `unset PWD`.

---

## `export` (create, update, or list)

Two modes.

**No arguments:** print the environment, sorted, in `declare -x` format. The sort
is done on a *copy of the pointer array* so the real environment order is never
disturbed:

```c
copy = malloc(sizeof(char *) * (count + 1));   // copy the pointers, not the strings
... copy each pointer ...
sort_str_array(copy, count);                    // bubble sort the copy
```

Each entry prints as `declare -x KEY="VALUE"` (or just `declare -x KEY` if it has
no `=`).

**With arguments:** validate and set each one. A valid identifier starts with a
letter or `_` and continues with letters, digits, or `_`:

```c
if (!is_valid_identifier(arg))
    return (print_error("export", arg, "not a valid identifier"), 1);
eq = ft_strchr(arg, '=');
if (eq == NULL)                                 // export FOO  (no value)
    ... mark it: set to "" only if it doesn't already exist ...
else
    env_set(&shell->envp, key, eq + 1);         // export FOO=bar
```

`export FOO=bar` sets it; `export FOO` marks the name without a value; `export
1bad` is rejected with status 1. All the actual mutation goes through
[`env_set`](../env/README.md).

---

## `unset` (remove variables)

Walk the arguments and remove each from the environment:

```c
i = 1;
while (cmd->argv[i])
    env_unset(&shell->envp, cmd->argv[i++]);
return (0);
```

[`env_unset`](../env/README.md) is a no-op for names that don't exist, so
`unset NOPE` quietly succeeds, matching bash.

---

## `env` (print the environment)

```c
while (shell->envp[i])
    ft_putendl_fd(shell->envp[i++], STDOUT_FILENO);
```

Prints every `KEY=VALUE` in internal order (not sorted; that's `export`'s job).

---

## `exit` (leave the shell)

Print `exit`-style errors, clean up, and terminate the process:

```c
if (!cmd->argv[1])
    status = shell->last_status;                  // exit with $?
else if (!is_numeric(cmd->argv[1]))
{
    print_error("exit", cmd->argv[1], "numeric argument required");
    shell_free(shell);
    exit(2);
}
else if (cmd->argv[2])
    return (print_error("exit", NULL, "too many arguments"), 1);  // don't exit!
else
    status = ft_atoi(cmd->argv[1]);
shell_free(shell);
exit(status);
```

Three cases match bash exactly:

- `exit` → leave with the current `$?`.
- `exit abc` → `numeric argument required`, exit **2**.
- `exit 1 2` → `too many arguments`, and **stays running** (returns 1, no exit).

Note `exit` calls `shell_free(shell)` before `exit()` so the shell's own memory
is released even though the process is about to die, so valgrind stays clean.

---

## The files

| File | Builtin(s) |
|---|---|
| `echo.c` | `ft_echo` |
| `cd.c` | `ft_cd` |
| `pwd.c` | `ft_pwd` |
| `export.c` | `ft_export`, identifier validation |
| `export_print.c` | `print_export`: the sorted `declare -x` listing |
| `unset.c` | `ft_unset` |
| `env.c` | `ft_env` |
| `exit.c` | `ft_exit`, numeric check |

---

## Memory

Built-ins allocate little and clean up after themselves. `export`'s sort frees
its pointer-copy array (the strings belong to the real env and are untouched).
`export FOO=bar` frees its temporary `key`. `cd`'s `env_set` calls may replace
env strings, handled inside [env/](../env/README.md). Only `exit` frees the
whole `t_shell`, and only because it's about to end the process.
