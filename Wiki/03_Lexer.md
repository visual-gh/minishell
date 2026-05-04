# 03 — Lexer

Turn a raw line into a list of tokens. One pass, character-by-character.

## Metacharacters (only these, per subject)

```
space, tab    →  word separators (when unquoted)
'             →  single quote
"             →  double quote
|             →  TOK_PIPE
<             →  TOK_REDIR_IN  (or TOK_HEREDOC if doubled)
>             →  TOK_REDIR_OUT (or TOK_APPEND  if doubled)
```

Everything else is part of a WORD.

`\`, `;`, `&`, `(`, `)` are NOT special — treat as ordinary chars (the subject says you don't need to handle them; bash would error on syntax — for minishell, they go inside words).

## Quote rules

| Quote | Inside it... |
|---|---|
| `'...'` | NOTHING is interpreted. `$`, spaces, `|` are literal. |
| `"..."` | Spaces, `|`, `<`, `>` are literal. **`$` still expands.** |

Quotes themselves are removed by the expander, NOT the lexer. The lexer keeps them in `value` but tags `quoted` so the expander knows which spans were quoted.

> Simpler alternative: have the lexer emit fragments and tag each with its quote state, then concatenate adjacent WORDs at parse time. Pick one style and commit.

## Algorithm (single pass)

```
i = 0
while line[i]:
    skip unquoted whitespace
    if line[i] == '|': push TOK_PIPE; i++
    elif line[i] == '<':
        if line[i+1] == '<': push TOK_HEREDOC;  i += 2
        else:                push TOK_REDIR_IN; i++
    elif line[i] == '>':
        if line[i+1] == '>': push TOK_APPEND;    i += 2
        else:                push TOK_REDIR_OUT; i++
    else:
        read_word(&i)   // collects WORD until unquoted metachar
```

### read_word

```
state = NONE         // NONE | SQ | DQ
start = i
while line[i]:
    if state == NONE and is_metachar(line[i]) and not whitespace:
        break        // stop before pipe/redir
    if state == NONE and is_whitespace(line[i]):
        break
    if line[i] == '\'' and state != DQ:
        state = (state == SQ ? NONE : SQ)
    elif line[i] == '"' and state != SQ:
        state = (state == DQ ? NONE : DQ)
    i++
push TOK_WORD with substring [start..i)
```

Track the `quoted` flag on the token — set it to `1` if any quote char appeared while building this WORD, otherwise `0`. The expander walks the string itself and tracks per-character quote state; the lexer's flag exists only so the expander can decide whether an empty post-expansion word survives (`""` → keep, unquoted empty → drop).

## Unclosed quote = error

If you exit `read_word` with `state != NONE`, the line had an unclosed quote.

Bash: prints `> ` and waits for more input. Minishell: subject says **don't interpret unclosed quotes** — just print an error to stderr and set `last_status = 2`.

```
minishell: syntax error: unexpected end of input (unclosed quote)
```

## Examples

| Input | Tokens |
|---|---|
| `ls -la` | WORD(`ls`) WORD(`-la`) |
| `cat<file` | WORD(`cat`) TOK_REDIR_IN WORD(`file`) |
| `echo "hi"` | WORD(`echo`) WORD(`"hi"`, quoted=1) |
| `echo a"b"c` | WORD(`echo`) WORD(`a"b"c`, quoted=1) |
| `cat<<EOF` | WORD(`cat`) TOK_HEREDOC WORD(`EOF`) |
| `'$USER'` | WORD(`'$USER'`, quoted=1) |
| `"$USER"` | WORD(`"$USER"`, quoted=1) |

Note: `a"b"c` is **one** token, not three. Adjacent quoted/unquoted runs glue into a single WORD.

## Common bugs

- **Whitespace inside quotes splitting tokens.** Don't break on whitespace if `state != NONE`.
- **Treating `<<<`** like a here-string. It's not in the subject — let it be `<<` then `<` as separate tokens, and let the parser flag the syntax error.
- **Forgetting that `>` and `<` need a target.** That's a parser concern, not a lexer one. Lexer just emits the redir token.
- **Greedy `>>`/`<<`.** Always check the *next* char before committing to single-char redir.
