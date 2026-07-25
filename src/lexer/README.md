# src/lexer/: Tokenizing

The lexer is stage 1. It takes the raw line from `readline` and cuts it into a
list of [`t_token`](../../includes/README.md): words and operators. It answers
exactly one question about every character (*"is this part of a word, or is it
an operator, or is it whitespace between things?"*) and nothing more.

```
"echo hi > out.txt"
        │
   ┌────▼────┐
   │  LEXER  │
   └────┬────┘
        ▼
[WORD echo] → [WORD hi] → [REDIR_OUT] → [WORD out.txt]
```

What the lexer **does not** do is just as important: it does not expand `$VAR`,
it does not remove quotes, it does not check whether `out.txt` is a valid target.
It only finds boundaries. Keeping it that dumb is what makes the rest of the
project tractable. See the [expander](../expander/README.md) for who owns quotes.

---

## The main loop

`lexer` walks the string, skipping whitespace, and hands each chunk to
`handle_token`:

```c
while (input[i] != '\0')
{
    while (input[i] == ' ' || input[i] == '\t')   // skip blanks between tokens
        i++;
    if (input[i] == '\0')
        break ;
    if (handle_token(&head, &tail, &i, input) == -1)
    {
        free_tokens(head);     // one failure and we drop everything
        return (NULL);
    }
}
```

Each token type advances `i` by the right amount, so the loop naturally lands on
the start of the next token. Note the `&tail` passed alongside `&head`: that's a
tail pointer so appending is O(1). Without it, building the list would re-walk
from the head every time (O(n²) on a long line). `add_token` uses it:

```c
void add_token(t_token **head, t_token **tail, t_token *new_node)
{
    if (*head == NULL)
        *head = new_node;
    else
        (*tail)->next = new_node;   // jump straight to the end
    *tail = new_node;
}
```

---

## Recognising operators: `separator_type`

Given a position, this returns what kind of separator is there, peeking at the
*next* character to tell `<` from `<<`:

```c
int separator_type(char *str, int i)
{
    if (str[i] == '|')
        return (TOK_PIPE);
    else if (str[i] == '<' && str[i + 1] == '<')     // must check << before <
        return (TOK_HEREDOC);
    else if (str[i] == '>' && str[i + 1] == '>')
        return (TOK_APPEND);
    else if (str[i] == '<')
        return (TOK_REDIR_IN);
    else if (str[i] == '>')
        return (TOK_REDIR_OUT);
    else
        return (TOK_WORD);       // anything else starts a word
}
```

Order matters: `<<` has to be tested before `<`, or `<<` would read as two
`<` tokens. `lex_redir` then advances `i` by 2 for `<<`/`>>` and by 1 for the
single-char ones.

> The name says what it returns: a `t_tok_type`, not a yes/no. It was renamed
> from `is_separator` for exactly that reason: `is_` reads like a boolean.

---

## Reading a word: the quote-aware scan

This is the one clever function in the lexer. A word runs until whitespace or an
operator, **except** inside quotes, where spaces and operators are just ordinary
characters. `echo "a | b"` is two words (`echo` and `"a | b"`), not four, because
the `|` is inside quotes.

So `scan_word` walks forward tracking whether it's currently inside a quote:

```c
static int is_word_char(char *input, int i, char quote)
{
    if (quote)                              // inside quotes: everything counts
        return (1);
    if (input[i] == ' ' || input[i] == '\t')
        return (0);
    return (separator_type(input, i) == TOK_WORD);   // outside: stop at operators
}

static int scan_word(char *input, int i, int *err)
{
    char quote;

    quote = 0;
    while (input[i] && is_word_char(input, i, quote))
    {
        if (quote)                          // in a quote: only its twin closes it
        {
            if (input[i] == quote)
                quote = 0;
        }
        else if (input[i] == '\'' || input[i] == '"')
            quote = input[i];               // open a quote
        i++;
    }
    if (quote)                              // reached end still inside a quote
        *err = 1;
    return (i);
}
```

The crucial detail: `quote` is used **only to find where the word ends.** The
characters (quotes included) are copied into the token's `value` verbatim with
`ft_substr`. The token for `"a | b"` literally contains the two quote characters.
The expander strips them later. This single decision is why quoting works
correctly across the whole shell instead of being half-handled in two places.

---

## Unclosed quotes

If `scan_word` reaches the end of the line while still inside a quote, it sets
`err`, and `lex_word` turns that into a hard failure:

```c
if (err)
{
    print_error(NULL, NULL, "unclosed quote");
    return (-1);
}
```

That `-1` propagates up: `lexer` frees the partial list and returns `NULL`,
`process_line` sees no tokens and sets `$?` to 2, matching bash, which rejects
`echo "hi` the same way. The subject explicitly says an unclosed quote is not
something we interpret, so refusing the line is correct.

---

## The files

| File | What's in it |
|---|---|
| `lexer.c` | `lexer` (the main loop), `handle_token` dispatch, `separator_type` |
| `lexer_scan.c` | `scan_word` and `lex_word`, the quote-aware word reader |
| `lexer_utils.c` | `lex_pipe`, `lex_redir`: emit an operator token, advance `i` |
| `token_init.c` | `token_init` (allocate one token), `add_token` (tail append) |

---

## Memory

Every token owns its `value` (a `ft_strdup` of the raw text). `free_tokens`
walks the list freeing each `value` then each node. It runs right after the
[parser](../parser/README.md) is done: the parser copies what it needs into the
[command list](../../includes/README.md), so tokens are short-lived scaffolding.
On any allocation failure mid-build, the whole partial list is freed and the
lexer returns `NULL`; nothing is left dangling.

**Next stage:** the [parser](../parser/README.md) turns this flat token list into
a list of commands.
