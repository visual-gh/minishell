# 04 — Parser

Token list → linked list of `t_cmd`. One linear walk, no recursion needed for mandatory.

## Grammar (informal)

```
pipeline   := command ( '|' command )*
command    := (redir | WORD)+
redir      := ('<' | '>' | '<<' | '>>') WORD
```

Every command has at least one element (a redir or a word). An empty command between two pipes is a syntax error.

## Algorithm

```
cur = new_cmd()                  // head of cmd list
shell->cmds = cur
for each token t:
    if t->type == TOK_PIPE:
        if cmd_empty(cur) || !t->next: syntax error
        cur->next = new_cmd()
        cur = cur->next
    elif is_redir(t->type):
        if !t->next || t->next->type != TOK_WORD: syntax error
        append_redir(cur, t, t->next)
        skip the next token (it was the target)
    else (TOK_WORD):
        append_argv(cur, t->value)
NULL-terminate every argv
```

`new_cmd()` returns a calloc'd `t_cmd`. `append_redir(cmd, redir_tok, target_tok)` builds a `t_redir` node and tail-appends to `cmd->redirs`. `append_argv(cmd, value)` grows `cmd->argv` by one (realloc with NULL-terminator) — or build an intermediate list of strings during parsing and flatten to `char **` at the end (cleaner under the Norm: list grows freely, single allocation at the end).

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

## Why a linked list works

Mandatory pipelines are linear: `c1 | c2 | c3 | c4`. No nesting. The natural shape is a forward-linked list. Each `t_cmd` carries:

- its own `argv` (for `execve`)
- its own `redirs` list (applied in textual order before `exec`)
- a `next` pointer to the following command in the pipeline

Pipeline stitching is the executor's job: it walks the list once to count commands, allocates `n - 1` pipes and `n` pids, then walks again to fork.

## Redirection order matters

```sh
> a > b   # opens a (truncates), opens b (truncates), final stdout = b
> b > a   # opens b, opens a, final stdout = a
```

Apply redirs in the order they appeared. Each `>` truncates its target file even if a later redir takes over the fd. (This is observable behavior — bash creates *both* files.)

The list preserves insertion order naturally: append at tail during parsing, walk head-to-tail during apply.

## Heredoc detection

When the parser sees `TOK_HEREDOC`, it stores the delimiter in a `t_redir` with `type = REDIR_HEREDOC` and the delimiter token's `quoted` flag copied to the redir's `quoted`. The body is **not** read here — that happens in the dedicated heredoc stage (`read_heredocs`) before the executor forks.

Implementation choice for body delivery: write the body into an unnamed pipe in the parent and hand the read end to the child as fd 0. See `06_Executor.md`.

## Pseudocode in C-ish form

```c
int  parse(t_token *tokens, t_shell *sh)
{
    t_cmd	*cur;

    cur = new_cmd();
    if (!cur)
        return (1);
    sh->cmds = cur;
    while (tokens)
    {
        if (tokens->type == TOK_PIPE)
        {
            if (cmd_empty(cur) || !tokens->next)
                return (syntax_err("|"));
            cur->next = new_cmd();
            cur = cur->next;
        }
        else if (is_redir(tokens->type))
        {
            if (!tokens->next || tokens->next->type != TOK_WORD)
                return (syntax_err(redir_str(tokens->type)));
            append_redir(cur, tokens, tokens->next);
            tokens = tokens->next;
        }
        else
            append_argv(cur, tokens->value);
        tokens = tokens->next;
    }
    return (0);
}
```

Norm split: `parse`, `new_cmd`, `append_redir`, `append_argv`, `cmd_empty` — five functions, fits in one `parser.c`. Syntax helpers (`syntax_err`, `redir_str`, `is_redir`) go in `parser_syntax.c`.

## Memory rules for the parser

- Every node allocated must be freed by `free_cmd_list` on success or on error.
- On syntax error: free `sh->cmds`, set `sh->cmds = NULL`, set `last_status = 2`, return non-zero.
- `argv` strings are duplicated from the token's `value` (or moved — pick one and stick to it). If moved, the lexer's `free_tokens` must not free those slots; mark them as taken or keep the duplicate path for simplicity.
