# 04 — Parser

Token list → flat `t_cmd[]` array. One linear walk, no recursion needed for mandatory.

## Grammar (informal)

```
pipeline   := command ( '|' command )*
command    := (redir | WORD)+
redir      := ('<' | '>' | '<<' | '>>') WORD
```

Every command has at least one element (a redir or a word). An empty command between two pipes is a syntax error.

## Algorithm

```
n_cmds = count(PIPE) + 1
allocate cmds[n_cmds]
i = 0  // cmd index
for each token:
    if token is PIPE:
        finish current cmd; i++; continue
    if token is REDIR_*:
        next must be WORD (else syntax error)
        append redir to cmds[i].redirs
        consume next WORD
    else (WORD):
        append to cmds[i].argv
finalize last cmd
NULL-terminate every argv
```

## Syntax errors to catch

Print to stderr, set `last_status = 2`, **abort the line** (don't execute any of it):

| Input | Error |
|---|---|
| `\|` | `syntax error near unexpected token '\|'` |
| `\| ls` | leading pipe — same |
| `ls \|` | trailing pipe — same |
| `ls \|\| ls` | empty command between pipes |
| `ls >` | `syntax error near unexpected token 'newline'` |
| `> < file` | `syntax error near unexpected token '<'` |
| `cat <<` | missing heredoc delimiter |

Quick rule: a redir token must be followed by a WORD. A pipe must be preceded and followed by a non-empty command.

## Why a flat table works

Mandatory pipelines are linear: `c1 | c2 | c3 | c4`. No nesting. So the natural shape is a left-to-right array. Each `t_cmd` carries:

- its own `argv` (for `execve`)
- its own `redirs[]` (applied in textual order before `exec`)

The pipeline stitching is the executor's job: it creates `n_cmds-1` pipes and `dup2`s child fds.

## Redirection order matters

```sh
> a > b   # opens a (truncates), opens b (truncates), final stdout = b
> b > a   # opens b, opens a, final stdout = a
```

Apply redirs in the order they appeared. Each `>` truncates its target file even if a later redir takes over the fd. (This is observable behavior — bash creates *both* files.)

## Heredoc detection

If a command has any `<<` redir, the heredoc must be **read at parse time** (or at least *before* forking the pipeline), because:

- Heredoc reads from the user's terminal interactively.
- It must finish before child execution starts.

Implementation choice: collect heredoc bodies into temp files (e.g., `/tmp/.minishell_heredoc_<pid>_<n>`) and rewrite the redir to a normal `<`. Or pipe them to the child via an unnamed pipe written by the parent. See `06_Executor.md`.

## Pseudocode in C-ish form

```c
int  parse(t_token *tokens, t_shell *sh)
{
    int n = count_cmds(tokens);
    sh->cmds = ft_calloc(n, sizeof(t_cmd));
    sh->n_cmds = n;
    int  i = 0;
    while (tokens)
    {
        if (tokens->type == TOK_PIPE)
        {
            if (cmd_empty(&sh->cmds[i]) || !tokens->next)
                return (syntax_err("|"));
            i++;
        }
        else if (is_redir(tokens->type))
        {
            if (!tokens->next || tokens->next->type != TOK_WORD)
                return (syntax_err(redir_str(tokens->type)));
            push_redir(&sh->cmds[i], tokens, tokens->next);
            tokens = tokens->next;
        }
        else
            push_argv(&sh->cmds[i], tokens->value);
        tokens = tokens->next;
    }
    return (0);
}
```

Norm split: `parse`, `count_cmds`, `push_redir`, `push_argv`, `cmd_empty` — five functions, fits in one `parser.c`. Syntax helpers go in `parser_syntax.c`.
