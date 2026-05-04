# 02 — Architecture

## The pipeline

```
LEXER → PARSER → HEREDOC → EXPANDER → EXECUTOR
```

One pass per stage. No backtracking. Each stage owns one transformation. If a stage detects a fatal problem (syntax error, malloc fail), it sets `last_status` and aborts the line; the outer loop reads the next prompt.

Why heredoc has its own stage between parser and expander: heredoc bodies are read interactively from the user **before** the pipeline forks. That gives Ctrl-C a clean abort point and keeps the executor unaware of terminal input. Heredoc body expansion (`$VAR` inside the body) happens at read time using the delimiter's `quoted` flag, not later in the main expander pass.

## Why no AST?

Bonus operators (`&&`, `||`, parentheses) demand a tree. Mandatory only needs:

- one or more commands
- joined by pipes (`|`)
- each with redirections

That's a **linear list of commands**. Cleaner, smaller, faster to write, easier to defend.

## Why linked lists (not arrays)?

Both shapes work. Linked lists win for this project:

- **One-pass parse.** Append a node when you see a token. No pre-counting, no `realloc`.
- **Reuses libft muscle memory.** `ft_lstnew` / `ft_lstadd_back` / `ft_lstclear` patterns transfer directly.
- **Norm-friendly.** Append/free are 5–10 line helpers; arrays push toward bigger functions tracking capacity and growth.
- **No VLA temptation.** `int pipes[n-1][2]` is forbidden by the Norm anyway, so the executor mallocs runtime arrays from a counted list — same code as it'd be either way.

The one place arrays would be marginally simpler — pipeline indexing — costs about four extra lines (count the cmd list, malloc `pids`/`pipes`).

## Core data structures

### t_token
```c
typedef enum e_tok_type
{
	TOK_WORD,
	TOK_PIPE,
	TOK_REDIR_IN,
	TOK_REDIR_OUT,
	TOK_APPEND,
	TOK_HEREDOC,
}	t_tok_type;

typedef struct s_token
{
	t_tok_type		type;
	char			*value;     /* raw text, quote chars still inside */
	int				quoted;     /* 1 if any quote char was in the word */
	struct s_token	*next;
}	t_token;
```

End of token list is `next == NULL`. No sentinel.

The `quoted` field is used by the **expander** to decide whether to keep an empty unquoted word (drop it) versus an empty quoted word (`""` — keep it as an empty argv slot). Per-character quote state is reconstructed during expansion by walking the value, not stored on the token.

### t_redir
```c
typedef enum e_redir_type
{
	REDIR_IN,        /* <  file */
	REDIR_OUT,       /* >  file (truncate) */
	REDIR_APPEND,    /* >> file */
	REDIR_HEREDOC,   /* << delim */
}	t_redir_type;

typedef struct s_redir
{
	t_redir_type	type;
	char			*target;    /* filename or heredoc delimiter */
	int				quoted;     /* heredoc: 1 if delim was quoted */
	struct s_redir	*next;
}	t_redir;
```

### t_cmd
```c
typedef struct s_cmd
{
	char			**argv;     /* NULL-terminated, post-expansion */
	t_redir			*redirs;    /* applied in order, left-to-right */
	struct s_cmd	*next;
}	t_cmd;
```

### t_shell
```c
typedef struct s_shell
{
	char			**envp;       /* owned, mutable copy of environment */
	t_cmd			*cmds;        /* current pipeline (head of list) */
	int				last_status;  /* $? */
}	t_shell;
```

`t_shell` is passed by pointer everywhere. **Never** put it in a global. The only global allowed is `g_signal` — a `volatile sig_atomic_t` storing the last received signal number.

## Ownership / lifetime rules

Per line entered:
1. `readline()` returns a `char *line` → owned by us, must `free()` after use.
2. Lexer allocates a token list → freed after parser builds cmds.
3. Parser allocates a `t_cmd` list, each with its own `t_redir` list and argv.
4. Heredoc reader allocates body buffers (or pipes), tied to the redir node.
5. Expander mutates strings in-place or replaces them (free old, set new).
6. Executor reads-only from cmds; only the parent frees them after `waitpid`.

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
    ├── heredoc/
    │   └── heredoc.c
    ├── expander/
    │   ├── expander.c
    │   └── expander_var.c
    ├── executor/
    │   ├── exec.c
    │   ├── exec_pipe.c
    │   ├── exec_redir.c
    │   └── exec_path.c
    ├── builtins/
    │   ├── builtin_echo.c
    │   ├── builtin_cd.c
    │   ├── builtin_pwd.c
    │   ├── builtin_export.c
    │   ├── builtin_unset.c
    │   ├── builtin_env.c
    │   └── builtin_exit.c
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

## Public interface (from `minishell.h`)

The header is the contract every stage agrees on. Headlines:

```c
/* lexer */
t_token *lex(char *line);
void    free_tokens(t_token *tokens);

/* parser */
int     parse(t_token *tokens, t_shell *shell);

/* heredoc — reads bodies before fork */
int     read_heredocs(t_shell *shell);

/* expander */
int     expand_cmds(t_shell *shell);

/* executor */
int     execute(t_shell *shell);

/* signals (4 modes) */
void    signals_prompt(void);    /* REPL waiting on readline */
void    signals_wait(void);      /* parent waiting on a child */
void    signals_child(void);     /* child process before execve */
void    signals_heredoc(void);   /* reading heredoc body */

/* errors / free */
int     print_error(char *cmd, char *arg, char *msg);
void    free_cmd_list(t_cmd *cmds);
void    free_redirs(t_redir *redirs);
void    free_str_array(char **arr);
```

The header order maps to the runtime call order — read it top-to-bottom and you can wire the REPL.
