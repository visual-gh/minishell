# includes/: The data model

`minishell.h` is the single header every `.c` file includes. It holds the system
includes, the one global, the type definitions, and every function prototype.
The prototypes are just a phone book; the interesting part, and the thing to
understand before reading any code, is the **four structs**. They *are* the
project. Every stage of the pipeline exists to build one of them or turn one
into the next.

---

## Read these four in order

### `t_token`: what the lexer produces

A token is one meaningful piece of the command line: a word, or an operator.

```c
typedef enum e_tok_type
{
    TOK_WORD,        // ls, echo, "hi $USER", a filename...
    TOK_PIPE,        // |
    TOK_REDIR_IN,    // <
    TOK_REDIR_OUT,   // >
    TOK_APPEND,      // >>
    TOK_HEREDOC,     // <<
}   t_tok_type;

typedef struct s_token
{
    t_tok_type      type;    // which of the above
    char            *value;  // the raw text, for WORDs only (operators are NULL)
    struct s_token  *next;   // singly linked list
}   t_token;
```

The line `cat < file | wc` becomes five tokens:

```
[WORD "cat"] → [REDIR_IN, value NULL] → [WORD "file"] → [PIPE, NULL] → [WORD "wc"]
```

Note `value` holds the **raw** text, quotes and all: the word `"hi $USER"` is
stored *with* its quotes. That is on purpose: the [expander](../src/expander/README.md)
is the only stage allowed to interpret quotes, so the lexer keeps its hands off.

### `t_redir`: one redirection

The parser detaches redirections from the token stream and hangs them off the
command they belong to.

```c
typedef struct s_redir
{
    t_redir_type    type;         // REDIR_IN / REDIR_OUT / REDIR_APPEND / REDIR_HEREDOC
    char            *target;      // the filename, or the heredoc delimiter
    int             quoted;       // was the delimiter quoted?  (heredoc only)
    int             heredoc_fd;   // read end of the heredoc pipe, or -1
    struct s_redir  *next;        // a command can have many redirs
}   t_redir;
```

Two fields exist purely for heredocs:

- **`quoted`** answers "was the `<<` delimiter written with quotes?", i.e.
  `<< "EOF"` vs `<< EOF`. It decides whether the heredoc body expands `$VAR`.
  The [parser](../src/parser/README.md) sets it via `strip_quotes`.
- **`heredoc_fd`** is filled in later by the [heredoc stage](../src/heredoc/README.md),
  which reads the body into a pipe and stores the pipe's read end here.

### `t_cmd`: one command in a pipeline

```c
typedef struct s_cmd
{
    char            **argv;    // NULL-terminated: { "wc", "-c", NULL }
    t_redir         *redirs;   // this command's redirections, or NULL
    struct s_cmd    *next;     // next command in the pipeline
}   t_cmd;
```

A pipeline is a linked list of these. `echo hi | cat | wc` is three `t_cmd`s.
A single command is a list of length one; the [executor](../src/executor/README.md)
treats "one command" and "a pipeline" as the same shape, which keeps it simple.

`argv` is exactly what `execve` wants, which is not an accident: it means the
executor can hand `cmd->argv` straight to `execve` with zero conversion.

### `t_shell`: everything the shell carries

```c
typedef struct s_shell
{
    char            **envp;         // our own heap copy of the environment
    t_cmd           *cmds;          // the command list for the current line
    int             last_status;    // $?: exit code of the last command
}   t_shell;
```

One `t_shell` is created at startup and threaded through almost every function.
`cmds` is rebuilt and freed for each line; `envp` and `last_status` live for the
whole session. See [env/](../src/env/README.md) for the environment and
[signals/](../src/signals/README.md) for who writes `last_status = 130`.

---

## The one global

```c
extern volatile sig_atomic_t    g_signal;
```

The subject allows **exactly one** global variable, and it may hold nothing but
a received signal number. This is it. A signal handler cannot safely touch
`t_shell`, so it drops the signal number here and the main loop reads it back.
The full reasoning (and why `volatile sig_atomic_t` specifically) is in
[signals/](../src/signals/README.md).

> Defense note: be ready to explain why this is the *only* global, and why it
> can't be a struct. "The handler runs in an async context; the only thing the
> C standard guarantees it can touch is a `volatile sig_atomic_t`, so anything
> richer would be unsafe" is the answer.

---

## Why one header, not one per module

The Norm forbids declaring a struct in a `.c` file, and these types cross every
boundary: the executor needs `t_cmd`, which the parser builds, which references
`t_token` from the lexer. A single shared header is the simplest correct choice.
The `# include`s are grouped at the top; the prototypes are grouped by stage with
banner comments that mirror the [directory layout](../src/README.md).
