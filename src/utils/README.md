# src/utils/: Error printing and cleanup

The plumbing every other module leans on: one function for printing errors the
bash way, and the family of `free_*` functions that keep the shell leak-free.
Nothing clever here, but the free functions are worth understanding, because
they encode *who owns what*, which is the thing that keeps a project this size
from leaking.

---

## `error.c`: `print_error`

One consistent way to write an error to stderr, so the whole shell speaks with
the same voice:

```c
int print_error(char *cmd, char *arg, char *msg)
{
    ft_putstr_fd("minishell: ", 2);      // always the program name
    if (cmd)                              // optional: the builtin/command
        ft_putstr_fd(cmd, 2), ft_putstr_fd(": ", 2);
    if (arg)                              // optional: the offending argument
        ft_putstr_fd(arg, 2), ft_putstr_fd(": ", 2);
    if (msg)
        ft_putstr_fd(msg, 2);
    ft_putstr_fd("\n", 2);
    return (1);                           // handy: return print_error(...) as a status
}
```

The three optional pieces compose into bash-style messages:

| Call | Output |
|---|---|
| `print_error(NULL, NULL, "unclosed quote")` | `minishell: unclosed quote` |
| `print_error("cd", "/nope", strerror(errno))` | `minishell: cd: /nope: No such file or directory` |
| `print_error("export", "1x", "not a valid identifier")` | `minishell: export: 1x: not a valid identifier` |

It returns `1` on purpose, so callers can write
`return (print_error(...), 1);` in a single line, a small idiom you'll see all
over the [builtins](../builtins/README.md).

> Redirection errors are the one exception: they use `perror` (to get the exact
> errno text) with a manual `minishell: ` prefix; see [executor/](../executor/README.md).

---

## `free.c`: the cleanup family

Each structure the shell builds has a matching `free_*`. They mirror the
[data model](../../includes/README.md) exactly:

```c
void free_str_array(char **arr);    // a NULL-terminated char ** (argv, envp, split)
void free_redirs(t_redir *redirs);  // one command's redirection list
void free_cmd_list(t_cmd *cmds);    // the whole command list
void free_tokens(t_token *head);    // the lexer's token list
void shell_free(t_shell *shell);    // everything: the top-level teardown
```

The interesting ones nest, freeing children before parents:

```c
void free_cmd_list(t_cmd *cmds)
{
    while (cmds)
    {
        next = cmds->next;
        free_str_array(cmds->argv);    // free the argv strings + array
        free_redirs(cmds->redirs);     // free each redir's target + node
        free(cmds);                    // then the command itself
        cmds = next;
    }
}

void shell_free(t_shell *shell)
{
    free_cmd_list(shell->cmds);        // commands (with their argv + redirs)
    free_str_array(shell->envp);       // the environment
    free(shell);                       // the struct
}
```

`free_str_array` guards against `NULL` (so freeing a command with no `argv` is
safe), and `free_redirs` frees each `target` before its node. Nothing is freed
twice because each structure has exactly one owner and one `free_*` that owns it.

---

## Where each free happens

This is the ownership map, the reason the shell doesn't leak:

| Structure | Built by | Freed by | When |
|---|---|---|---|
| token list | [lexer](../lexer/README.md) | `free_tokens` | right after `parse`, every line |
| command list | [parser](../parser/README.md) | `free_cmd_list` | after `execute`, every line |
| `argv` / redir strings | parser + [expander](../expander/README.md) | inside `free_cmd_list` | with their command |
| environment | [env](../env/README.md) | `free_str_array` | once, at shutdown |
| `t_shell` | `shell_init` | `shell_free` | once, at shutdown (or in `exit`) |

The rhythm per line is: lex → parse → **free tokens** → run → **free command
list**. The environment and the shell struct live for the whole session. Verify
the whole thing holds with:

```bash
printf 'export A=1\nls | wc -l\ncd /tmp\nexit\n' \
  | valgrind --leak-check=full ./minishell
```

The only reported leaks should be inside `readline`, which the subject exempts.

---

## The files

| File | What's in it |
|---|---|
| `error.c` | `print_error` |
| `free.c` | `free_str_array`, `free_redirs`, `free_cmd_list`, `free_tokens`, `shell_free` |

For the big picture of how these pieces fit the pipeline, see the
[src/ overview](../README.md).
