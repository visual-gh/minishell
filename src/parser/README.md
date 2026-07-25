# src/parser/: Building the command list

The parser is stage 2. It takes the flat [token list](../lexer/README.md) from
the lexer and turns it into a list of [`t_cmd`](../../includes/README.md):
grouping words into commands, splitting on pipes, and pulling redirections out of
the stream onto the command they belong to.

```
[WORD cat] [REDIR_IN] [WORD f] [PIPE] [WORD wc] [WORD -l]
        │
   ┌────▼─────┐
   │  PARSER  │
   └────┬─────┘
        ▼
cmd A: argv = { "cat" },  redirs = [ IN → "f" ]
cmd B: argv = { "wc", "-l" }
```

A pipe becomes the boundary between two `t_cmd`s. A redirection operator and the
word after it become one `t_redir` on the current command, and disappear from
`argv`. Everything else is a word in `argv`.

---

## The shape of the loop

`parse` builds one command per pipe segment:

```c
while (1)
{
    cmd = cmd_new();
    add_cmd(&shell->cmds, &tail, cmd);   // tail pointer → O(1) append
    if (parse_cmd(cmd, &tok) == -1)
        return (-1);
    if (tok == NULL)                     // ran out of tokens
        break ;
    tok = tok->next;                     // step over the PIPE, start next cmd
}
```

Note `tok` is walked by pointer (`t_token **`) through the helpers, so each
function leaves the cursor exactly where the next one should pick up.

---

## Counting before allocating

Here's the one thing to understand about this parser. A command's words can be
**interrupted by redirections**:

```
echo hi > file there
```

is the command `echo` with args `hi` and `there`, plus a redirection to `file`.
The words `hi` and `there` are on opposite sides of the `> file`. If you
allocated `argv` when you hit the first word run and again at the second, you'd
lose the first run and leak it. (That was a real bug once; the fix is below.)

So the parser does **two passes**. First `count_all_words` counts every word up
to the next pipe, stepping *over* each redirection operator and its target:

```c
while (tok && tok->type != TOK_PIPE)
{
    if (tok->type == TOK_WORD)
    {
        count++;
        tok = tok->next;
    }
    else                        // a redirection: skip the operator AND its target
    {
        tok = tok->next;
        if (tok)
            tok = tok->next;
    }
}
```

Then `parse_cmd` allocates `argv` exactly once, and `fill_words` makes a single
pass that fills words and attaches redirs as it meets them:

```c
count = count_all_words(*tok);
if (count > 0)
{
    cmd->argv = malloc(sizeof(char *) * (count + 1));
    cmd->argv[count] = NULL;
}
fill_words(cmd, tok);           // fills argv[0..], calls parse_redir on operators
```

One allocation, no reallocs, no leaks, words in the right order even when a
redirection splits them.

---

## Attaching a redirection: `parse_redir`

When `fill_words` meets a redirection operator, it hands off to `parse_redir`,
which grabs the *next* token as the target:

```c
type = redir_type_from_tok((*tok)->type);
*tok = (*tok)->next;
if (*tok == NULL || (*tok)->type != TOK_WORD)   // nothing, or another operator?
{
    print_error(NULL, NULL, "syntax error near unexpected token `newline'");
    return (-1);
}
target = strip_quotes((*tok)->value, &quoted);
add_redir(cmd, type, target, quoted);
```

`> |` or a trailing `>` with no filename is a syntax error, exactly like bash.

### Why redirection targets get their quotes stripped *here*

Words in `argv` keep their quotes; the [expander](../expander/README.md) owns
that. But redirection targets are stripped right here in the parser. Why the
asymmetry?

Because `strip_quotes` does **two** jobs at once, and one of them the parser
needs immediately:

```c
char *strip_quotes(char *raw, int *quoted);   // returns the unquoted string,
                                              // and reports whether there were quotes
```

The `quoted` out-parameter answers *"was the `<<` delimiter written with
quotes?"* (`<< "EOF"` vs `<< EOF`), which the [heredoc stage](../heredoc/README.md)
needs to decide whether to expand the body. We can't get that answer after the
quotes are gone, so we strip and record it in the same step. The stripped target
still passes through `expand_word` later (for `$VAR` in a filename); running it
on an already-quote-free string is harmless, since there's nothing left to strip.

---

## Empty commands

After `parse_cmd` runs, if a segment produced neither words nor redirections, the
line had a stray pipe like `| ls` or `ls | | wc`:

```c
if (cmd->argv == NULL && cmd->redirs == NULL)
{
    print_error(NULL, NULL, "syntax error near unexpected token `|'");
    return (-1);
}
```

Any `-1` from the parser makes `process_line` set `$?` to 2 and skip execution.

---

## The files

| File | What's in it |
|---|---|
| `parser.c` | `parse`, `parse_cmd`, `fill_words`, `parse_redir`: the core flow |
| `cmd_init.c` | `cmd_new`, `add_cmd` (tail append), `count_all_words` (the counting pass) |
| `parser_utils.c` | `add_redir`, `redir_new`, and `strip_quotes` + its helpers |

---

## Memory

The parser copies token text into fresh strings (`ft_strdup` for each `argv`
entry, `strip_quotes` allocates the target). That's deliberate: it means the
[token list](../lexer/README.md) can be freed the instant parsing finishes, and
the command list stands on its own. Everything the parser builds lands on
`shell->cmds` and is freed by `free_cmd_list` after the line runs; see the
[free family](../utils/README.md).

**Next stages:** the [heredoc reader](../heredoc/README.md) and the
[expander](../expander/README.md) both work on this command list before the
[executor](../executor/README.md) runs it.
