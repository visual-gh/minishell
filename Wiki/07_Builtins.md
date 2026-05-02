# 07 — Builtins

Each builtin returns an `int` exit status. Signature:

```c
int bi_<name>(t_shell *sh, char **argv);   // argv[0] = "name"
```

Dispatch table (or a chain of `if`/`else if`):

```c
int run_builtin(t_shell *sh, char **argv)
{
    if (!ft_strcmp(argv[0], "echo"))   return bi_echo(sh, argv);
    if (!ft_strcmp(argv[0], "cd"))     return bi_cd(sh, argv);
    if (!ft_strcmp(argv[0], "pwd"))    return bi_pwd(sh, argv);
    if (!ft_strcmp(argv[0], "export")) return bi_export(sh, argv);
    if (!ft_strcmp(argv[0], "unset"))  return bi_unset(sh, argv);
    if (!ft_strcmp(argv[0], "env"))    return bi_env(sh, argv);
    if (!ft_strcmp(argv[0], "exit"))   return bi_exit(sh, argv);
    return 1;
}

int is_builtin(const char *s) { /* same names */ }
```

## echo

`echo [-n] [args...]` — concat args with single spaces, append newline (unless `-n`).

The `-n` flag in bash also accepts `-nnnn`, `-n -n`, etc. — any leading run of `-n+` is treated as the flag. Mimic:

```c
int i = 1, nl = 1;
while (argv[i] && is_n_flag(argv[i])) { nl = 0; i++; }
while (argv[i]) {
    write(1, argv[i], ft_strlen(argv[i]));
    if (argv[i + 1]) write(1, " ", 1);
    i++;
}
if (nl) write(1, "\n", 1);
return 0;
```

`is_n_flag(s)`: starts with `-`, all subsequent chars are `n`, length ≥ 2.

## pwd

`getcwd(buf, size)` → write + newline. Returns 0 on success. Don't trust env `PWD`; use `getcwd`.

```c
char *cwd = getcwd(NULL, 0);   // glibc allocates for you
if (!cwd) { perror("pwd"); return 1; }
ft_putendl_fd(cwd, 1); free(cwd);
return 0;
```

## cd

Subject says: relative or absolute path only. No `cd -`, no `cd` with no args (or do — bash uses `$HOME`). Pick: support no-arg = `$HOME`, error if HOME unset.

```c
int bi_cd(t_shell *sh, char **argv)
{
    char *target;
    if (!argv[1])                 target = env_get(sh, "HOME");
    else if (argv[2])             return err("cd: too many arguments"), 1;
    else                          target = argv[1];
    if (!target)                  return err("cd: HOME not set"), 1;
    if (chdir(target) < 0)        return perr2("cd", target), 1;
    update_pwd(sh);               // PWD = old, OLDPWD = new (optional)
    return 0;
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
for (int i = 0; sh->envp[i]; i++)
    if (ft_strchr(sh->envp[i], '='))
        ft_putendl_fd(sh->envp[i], 1);
return 0;
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
int bi_exit(t_shell *sh, char **argv)
{
    if (!in_pipeline) ft_putendl_fd("exit", 2);
    if (!argv[1]) clean_and_exit(sh, sh->last_status);
    if (!is_numeric(argv[1])) {
        err("exit: numeric argument required");
        clean_and_exit(sh, 2);
    }
    if (argv[2]) return err("exit: too many arguments"), 1;
    long n = ft_atol(argv[1]);
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
