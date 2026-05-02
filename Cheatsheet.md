# Minishell — Cheatsheet

One-page reference. Detailed treatment lives in `Wiki/`.

## Pipeline

```
readline → LEXER → PARSER → EXPANDER → EXECUTOR → wait → loop
```

## Token types

`WORD | PIPE | REDIR_IN(<) | REDIR_OUT(>) | APPEND(>>) | HEREDOC(<<)`

## Quote semantics

| | inside `'...'` | inside `"..."` |
|---|---|---|
| `$VAR` | literal | **expanded** |
| spaces | literal | literal |
| `\| < >` | literal | literal |
| `'` `"` | the *other* quote is literal | the *other* quote is literal |

Quote chars are removed by expander, not lexer.

## Expansion targets

- argv elements
- redir targets (filenames)
- heredoc body lines (only if heredoc delimiter was unquoted)

## Exit codes

| Code | Meaning |
|---|---|
| 0 | success |
| 1 | generic builtin failure |
| 2 | syntax error / `exit` numeric error |
| 126 | found, not executable / is dir |
| 127 | command not found |
| 128+N | killed by signal N (130=SIGINT, 131=SIGQUIT) |

`status = WIFEXITED(s) ? WEXITSTATUS(s) : 128 + WTERMSIG(s);`

## Signals

| Mode | SIGINT | SIGQUIT | EOF |
|---|---|---|---|
| prompt | newline + redisplay, $?=130 | ignored | exit shell |
| running child | child dies, parent prints \n | child dies, "Quit (core dumped)" | — |
| heredoc | abort, $?=130 | ignored | warning, continue |

Global: `volatile sig_atomic_t g_signal;` — signal number only, nothing else.

## Builtins quick ref

| Builtin | Args | Notes |
|---|---|---|
| `echo` | `-n` | concat with spaces, newline unless -n |
| `cd` | one path or none | none → $HOME; chdir |
| `pwd` | none | getcwd + write |
| `export` | `KEY=VAL` or none | sorted `declare -x` output if no args |
| `unset` | `KEY...` | silent if missing |
| `env` | none | only entries with `=` |
| `exit` | `[n]` | n%256, no n → last_status |

Builtin runs in **parent** if standalone, in **child** if in pipeline.

## fork/exec template (child)

```c
signals_set_default();
apply_redirs(c);                    // open + dup2 + close
if (is_builtin(c->argv[0])) exit(run_builtin(sh, c));
char *p = resolve_path(c->argv[0], sh->envp);
if (!p) { not_found_msg(c->argv[0]); exit(127); }
execve(p, c->argv, sh->envp);
perror(p); exit(126);
```

## Pipeline (n cmds)

```
create n-1 pipes
for i in 0..n-1:
    fork → child:
        if i>0:   dup2(p[i-1][0], 0)
        if i<n-1: dup2(p[i  ][1], 1)
        close ALL pipe fds
        apply this cmd's own redirs (override)
        run as builtin or execve
parent: close ALL pipe fds → waitpid each → capture LAST child's status
```

## Redirection open flags

| Op | Flags |
|---|---|
| `<` | `O_RDONLY` |
| `>` | `O_WRONLY \| O_CREAT \| O_TRUNC`, mode 0644 |
| `>>` | `O_WRONLY \| O_CREAT \| O_APPEND`, mode 0644 |
| `<<` | read body to pipe/temp, dup2 read-end → 0 |

## PATH lookup rules

```
if strchr(cmd, '/'): use as-is, check access(F_OK)/X_OK
else: getenv("PATH"); split ':'; first match with X_OK; else 127
```

## Norm survival

- 25 lines / function, 5 functions / file
- 5 variables / function (use a context struct if needed)
- All declarations at top, no init-on-declare
- Tabs for indent, asterisk on identifier (`char *p`), single tab between return type and name
- No `for`/`switch`/`do-while`/`?:`/`goto`/VLA
- One global allowed (`g_signal`); everything else `static`/`const`
- Headers: include guards, no `.c` includes, no unused headers

## Build

```make
NAME    = minishell
CC      = cc
CFLAGS  = -Wall -Wextra -Werror
LDFLAGS = -lreadline
LIBFT   = libft/libft.a

SRCS = $(addprefix src/, main.c \
       lexer/lexer.c lexer/lexer_quotes.c lexer/lexer_utils.c \
       parser/parser.c parser/parser_redir.c parser/parser_syntax.c \
       expander/expander.c expander/expander_var.c \
       executor/exec.c executor/exec_pipe.c executor/exec_redir.c \
       executor/exec_path.c executor/exec_heredoc.c \
       builtins/bi_echo.c builtins/bi_cd.c builtins/bi_pwd.c \
       builtins/bi_export.c builtins/bi_unset.c builtins/bi_env.c \
       builtins/bi_exit.c \
       env/env_get.c env/env_set.c env/env_dup.c \
       signals/signals.c \
       utils/error.c utils/free.c)

OBJS = $(SRCS:.c=.o)

all: $(NAME)
$(NAME): $(OBJS) $(LIBFT)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) $(LDFLAGS) -o $(NAME)
$(LIBFT):
	$(MAKE) -C libft
clean:
	rm -f $(OBJS); $(MAKE) -C libft clean
fclean: clean
	rm -f $(NAME); $(MAKE) -C libft fclean
re: fclean all
.PHONY: all clean fclean re
```

## Test loop

```bash
norminette src includes
cc -Wall -Wextra -Werror ... # zero warnings
valgrind --leak-check=full --track-fds=yes \
         --suppressions=minishell.supp ./minishell
# diff against bash for every input from 10_EdgeCases.md
```

## 12-week roadmap

| Wk | Goal |
|---|---|
| 1 | env copy, REPL skeleton, signals stub |
| 2 | lexer (all token types, quote tracking) |
| 3 | parser (flat cmds, syntax errors) |
| 4 | expander ($VAR, $?, quote strip) |
| 5 | signals (3 modes, heredoc handler) |
| 6 | builtins minus export/exit |
| 7 | export + exit + edge cases |
| 8 | single-cmd executor + redirs |
| 9 | pipelines + heredoc fd plumbing |
| 10 | edge case sweep, error messages match bash |
| 11 | valgrind clean, norm clean |
| 12 | defense rehearsal, README, buffer |

## Minimum viable order

If pressed for time: **lexer → parser → executor (single cmd) → builtins (echo/pwd/cd/env) → expander → signals → pipes → redirs → heredoc → export/unset/exit → polish**.

Can demo something working end of week 4.
