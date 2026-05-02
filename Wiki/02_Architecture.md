# 02 — Architecture

## The pipeline

```
LEXER → PARSER → EXPANDER → EXECUTOR
```

One pass per stage. No backtracking. Each stage owns one transformation. If a stage detects a fatal problem (syntax error, malloc fail), it sets `last_status` and aborts the line; the outer loop reads the next prompt.

## Why no AST?

Bonus operators (`&&`, `||`, parentheses) demand a tree. Mandatory only needs:

- one or more commands
- joined by pipes (`|`)
- each with redirections

That's a **flat array of commands**. Cleaner, smaller, faster to write, easier to defend.

## Core data structures

### t_token
```c
typedef enum e_tok_type
{
	TOK_WORD,        // any unquoted/quoted text fragment
	TOK_PIPE,        // |
	TOK_REDIR_IN,    // <
	TOK_REDIR_OUT,   // >
	TOK_APPEND,      // >>
	TOK_HEREDOC,     // <<
	TOK_EOF
}	t_tok_type;

typedef struct s_token
{
	t_tok_type		type;
	char			*value;     // raw text, quote chars still inside
	int				quoted;     // 0=none, 1=single, 2=double, 3=mixed
	struct s_token	*next;
}	t_token;
```

The `quoted` field is crucial: the **expander** needs to know which characters were inside which quote to decide if `$VAR` should expand.

### t_redir
```c
typedef enum e_redir_type
{
	R_IN,        // <  file
	R_OUT,       // >  file (truncate)
	R_APPEND,    // >> file
	R_HEREDOC    // << delim
}	t_redir_type;

typedef struct s_redir
{
	t_redir_type	type;
	char			*target;   // filename or heredoc delimiter
	int				quoted;    // for heredoc: if delim was quoted, no expansion
}	t_redir;
```

### t_cmd
```c
typedef struct s_cmd
{
	char	**argv;      // NULL-terminated, post-expansion
	t_redir	*redirs;     // applied in order, left-to-right
	int		n_redirs;
}	t_cmd;
```

### t_shell
```c
typedef struct s_shell
{
	char	**envp;       // owned, mutable copy of environment
	int		last_status;  // $?
	t_cmd	*cmds;        // current pipeline
	int		n_cmds;
}	t_shell;
```

`t_shell` is passed by pointer everywhere. **Never** put it in a global. The only global allowed is `g_signal` — an `int` storing the last received signal.

## Ownership / lifetime rules

Per line entered:
1. `readline()` returns a `char *line` → owned by us, must `free()` after use.
2. Lexer allocates tokens → freed after parser builds cmds.
3. Parser allocates `t_cmd` array + `t_redir` arrays + argv slots.
4. Expander mutates strings in-place or replaces them (free old, set new).
5. Executor reads-only from cmds; only the parent frees them after `waitpid`.

After every iteration the only state surviving is `t_shell.envp` and `last_status`. Everything else is freed before the next prompt.

## File layout (suggested)

```
minishell/
├── Makefile
├── libft/
├── includes/
│   └── minishell.h
└── src/
    ├── main.c              # REPL loop
    ├── shell_init.c        # env copy, signal install
    ├── lexer/
    │   ├── lexer.c
    │   ├── lexer_quotes.c
    │   └── lexer_utils.c
    ├── parser/
    │   ├── parser.c
    │   ├── parser_redir.c
    │   └── parser_syntax.c
    ├── expander/
    │   ├── expander.c
    │   └── expander_var.c
    ├── executor/
    │   ├── exec.c
    │   ├── exec_pipe.c
    │   ├── exec_redir.c
    │   ├── exec_path.c
    │   └── exec_heredoc.c
    ├── builtins/
    │   ├── bi_echo.c
    │   ├── bi_cd.c
    │   ├── bi_pwd.c
    │   ├── bi_export.c
    │   ├── bi_unset.c
    │   ├── bi_env.c
    │   └── bi_exit.c
    ├── env/
    │   ├── env_get.c
    │   ├── env_set.c
    │   └── env_dup.c
    ├── signals/
    │   └── signals.c
    └── utils/
        ├── error.c
        └── free.c
```

Norm: **5 functions per .c max**, 25 lines per function. The split above leaves headroom.

## Error & exit code conventions

| Situation | Status |
|---|---|
| Success | `0` |
| Builtin generic failure | `1` |
| Misuse / bad syntax | `2` |
| `Ctrl-C` interrupted | `130` (128 + SIGINT) |
| `Ctrl-\` SIGQUIT | `131` (128 + SIGQUIT) |
| Command not found | `127` |
| Command found but not executable | `126` |
| Killed by signal N | `128 + N` |
| Normal child exit code N | `N` |

Always `WIFEXITED(s) ? WEXITSTATUS(s) : 128 + WTERMSIG(s)`.
