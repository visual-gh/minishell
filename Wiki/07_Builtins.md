# 07 — Builtins

Each builtin returns an `int` exit status. Signatures (from the header):

```c
int builtin_echo(t_cmd *cmd);
int builtin_cd(t_cmd *cmd, t_shell *shell);
int builtin_pwd(void);
int builtin_export(t_cmd *cmd, t_shell *shell);
int builtin_unset(t_cmd *cmd, t_shell *shell);
int builtin_env(t_shell *shell);
int builtin_exit(t_cmd *cmd, t_shell *shell);
```

Each builtin reads `cmd->argv` (NULL-terminated, post-expansion) and returns its exit status.

Dispatch:

```c
int run_builtin(t_cmd *cmd, t_shell *sh)
{
    char    *name;

    name = cmd->argv[0];
    if (!ft_strcmp(name, "echo"))   return (builtin_echo(cmd));
    if (!ft_strcmp(name, "cd"))     return (builtin_cd(cmd, sh));
    if (!ft_strcmp(name, "pwd"))    return (builtin_pwd());
    if (!ft_strcmp(name, "export")) return (builtin_export(cmd, sh));
    if (!ft_strcmp(name, "unset"))  return (builtin_unset(cmd, sh));
    if (!ft_strcmp(name, "env"))    return (builtin_env(sh));
    if (!ft_strcmp(name, "exit"))   return (builtin_exit(cmd, sh));
    return (1);
}

int is_builtin(char *name);   /* same names */
```

## echo

`echo [-n] [args...]` — concat args with single spaces, append newline (unless `-n`).

The `-n` flag in bash also accepts `-nnnn`, `-n -n`, etc. — any leading run of `-n+` is treated as the flag. Mimic:

```c
int     i;
int     newline;

i = 1;
newline = 1;
while (cmd->argv[i] && is_n_flag(cmd->argv[i]))
{
    newline = 0;
    i++;
}
while (cmd->argv[i])
{
    write(1, cmd->argv[i], ft_strlen(cmd->argv[i]));
    if (cmd->argv[i + 1])
        write(1, " ", 1);
    i++;
}
if (newline)
    write(1, "\n", 1);
return (0);
```

`is_n_flag(s)`: starts with `-`, all subsequent chars are `n`, length ≥ 2.

## pwd

`getcwd(buf, size)` → write + newline. Returns 0 on success. Don't trust env `PWD`; use `getcwd`.

```c
char    *cwd;

cwd = getcwd(NULL, 0);
if (!cwd)
    return (perror("pwd"), 1);
ft_putendl_fd(cwd, 1);
free(cwd);
return (0);
```

## cd

Subject says: relative or absolute path only. No `cd -`. With no args use `$HOME`; error if HOME unset.

```c
int     builtin_cd(t_cmd *cmd, t_shell *sh)
{
    char    *target;

    if (!cmd->argv[1])
        target = env_get(sh->envp, "HOME");
    else if (cmd->argv[2])
        return (print_error("cd", NULL, "too many arguments"), 1);
    else
        target = cmd->argv[1];
    if (!target)
        return (print_error("cd", NULL, "HOME not set"), 1);
    if (chdir(target) < 0)
        return (print_error("cd", target, strerror(errno)), 1);
    update_pwd(sh);
    return (0);
}
```

`update_pwd`: read `getcwd`, set `PWD`, save previous to `OLDPWD`. Bash does this; nice-to-have.

## export

`export` (no args) → print all vars in `declare -x KEY="VALUE"` form, **sorted**:
```
declare -x HOME="/home/visual"
declare -x PATH="/usr/bin:/bin"
```

`export FOO=bar` → set/update env var.
`export FOO` → mark as exported (in your envp, just add `FOO` with no `=`); printed by `export` but not by `env`.
`export 1FOO=x` → invalid identifier: `minishell: export: '1FOO': not a valid identifier`, status 1.

Identifier rule: `[a-zA-Z_][a-zA-Z0-9_]*`.

Two-tier env (with-`=` vs without-`=`) is the cleanest way; many minishells just store everything as `KEY=VALUE` and filter for `env` output.

## unset

`unset KEY [KEY ...]` — remove from envp. Silent if KEY doesn't exist. Reject invalid identifiers with the same error as export, status 1 only if any was invalid.

## env

No options/args. Print every `KEY=VALUE` line where the entry has a `=` (skip "exported but unassigned" entries). Don't print declared-only vars.

```c
int     i;

i = 0;
while (sh->envp[i])
{
    if (ft_strchr(sh->envp[i], '='))
        ft_putendl_fd(sh->envp[i], 1);
    i++;
}
return (0);
```

If invoked with args, bash treats them as `env [vars] CMD args` — but subject says **no options or arguments**. So if `argv[1]` exists, error and return non-zero. Match bash error message style.

## exit

`exit [n]`:
- no n → exit with `last_status`
- n is numeric → exit with `n & 0xff` (bash takes mod 256)
- n is non-numeric → `minishell: exit: <arg>: numeric argument required`, exit 2
- more than one arg → `minishell: exit: too many arguments`, return 1 **without exiting** (bash behavior)

Print `exit\n` to stderr **only when interactive and not in a pipeline** (when running as the parent's foreground command). `exit | cat` does NOT print "exit".

```c
int     builtin_exit(t_cmd *cmd, t_shell *sh)
{
    long    n;

    if (!in_pipeline)
        ft_putendl_fd("exit", 2);
    if (!cmd->argv[1])
        clean_and_exit(sh, sh->last_status);
    if (!is_numeric(cmd->argv[1]))
    {
        print_error("exit", cmd->argv[1], "numeric argument required");
        clean_and_exit(sh, 2);
    }
    if (cmd->argv[2])
        return (print_error("exit", NULL, "too many arguments"), 1);
    n = ft_atol(cmd->argv[1]);
    clean_and_exit(sh, (int)(n & 0xff));
}
```

`clean_and_exit`: free envp, free cmds/tokens if any, `rl_clear_history()`, then `exit()`.

## Builtins in pipeline vs parent

| Where it runs | Why |
|---|---|
| Standalone single command → parent | so `cd`, `export`, `unset`, `exit` modify shell state |
| Anywhere in a pipeline → forked child | dup2 would clobber parent fds |

Inside the child, the builtin still does its thing then `exit(status)`. State changes (export, cd) don't affect the parent — that's correct bash behavior.

## Common bugs

- Forgetting that `cd` in `cd dir | echo` doesn't actually change directory (it ran in a child).
- Mutating envp without freeing the old slot → leak.
- `export` printing in insertion order instead of sorted.
- `exit 9999999999999999999` → must overflow-check; bash treats unparseable bigint as numeric error.
- Missing the `exit\n` print to stderr.
