# src/expander/: Expansion and quote removal

The expander is stage 4 (after [heredocs](../heredoc/README.md) are read, before
the [executor](../executor/README.md) runs). It takes the still-raw words on the
[command list](../../includes/README.md) and turns them into their final values:
`$VAR` and `$?` are resolved, and quotes are removed.

Given:

```
echo "Hello $USER, in '$PWD'"
```

the parser handed us the middle word **with its quotes intact**:
`"Hello $USER, in '$PWD'"`. The expander rewrites it to
`Hello visual, in '$PWD'` and stores it back in `argv`, in place.

**The expander is the single owner of quoting in the whole project.** The lexer
deliberately kept quotes as literal characters; nobody else touches them. This
is the design decision that makes quoting correct: it lives in exactly one
place instead of being half-done in the lexer and half in the parser.

---

## The three quote rules

Everything here reduces to three rules:

| Context | `$` expanded? | Quotes shown? | Example → result |
|---|---|---|---|
| **single `'…'`** | no | no | `'$HOME'` → `$HOME` |
| **double `"…"`** | yes | no | `"$HOME"` → `/home/visual` |
| **unquoted** | yes | (n/a) | `$HOME` → `/home/visual` |

Single quotes are a "literal box": copy the contents verbatim. Double quotes are
almost the same but `$` still fires. Unquoted text behaves like double-quoted.

---

## A word is a sequence of segments

The key insight: a word like `"$HOME"/.config/'my app'` is not one thing to
expand; it's a run of segments, each under a different rule, concatenated:

| Segment | Rule | Result |
|---|---|---|
| `"$HOME"` | double | `/home/visual` |
| `/.config/` | plain | `/.config/` |
| `'my app'` | single | `my app` |
| **joined** | | `/home/visual/.config/my app` |

So `expand_word` walks the word and, at each position, dispatches on what it sees:

```c
while (word[i])
{
    if (word[i] == '\'')
        tmp = expand_single_quoted(word, &i);   // copy verbatim to closing '
    else if (word[i] == '"')
        tmp = expand_double_quoted(word, &i, shell);
    else if (word[i] == '$')
        tmp = expand_var(word, &i, shell);
    else
        tmp = scan_literal(word, &i, '\'', '"'); // plain run up to a quote or $
    res = strjoin_free(res, tmp);                // append, freeing the old res
    free(tmp);
}
```

Each branch returns a freshly built chunk and leaves `i` on the next segment.
`strjoin_free` joins the chunk onto the accumulator and frees the previous
accumulator, so the result grows cleanly with no manual bookkeeping.

### Grabbing whole runs, not single characters

`scan_literal` walks to the next character that *matters* and copies the entire
run in one `ft_substr`:

```c
static char *scan_literal(char *word, int *i, char stop1, char stop2)
{
    int start;

    start = *i;
    while (word[*i] && word[*i] != '$' && word[*i] != stop1 && word[*i] != stop2)
        (*i)++;
    return (ft_substr(word, start, *i - start));
}
```

This matters for performance. Appending one character at a time would re-copy the
whole accumulated string on every character, which is O(n²). On a 40 000-character word
that was measured at **6.73 s**; grabbing whole runs brought it to **0.11 s**.
The `stop1`/`stop2` parameters let the same function serve both the unquoted
scan (stop at `'` or `"`) and the inside-double-quotes scan (stop at `"`).

---

## Variable lookup: `expand_var`

Called whenever a `$` is found. It looks at what follows and decides:

```c
char *expand_var(char *str, int *i, t_shell *shell)
{
    (*i)++;                                     // step past the $
    if (!str[*i])
        return (ft_strdup("$"));                // trailing $ is a literal $
    if (str[*i] == '?')
    {
        (*i)++;
        return (ft_itoa(shell->last_status));   // $? → last exit code
    }
    if (!ft_isalpha(str[*i]) && str[*i] != '_')
        return (ft_strdup("$"));                // $5, "$ " → literal $
    return (expand_named_var(str, i, shell));   // $NAME → look up in env
}
```

| Input | Result |
|---|---|
| `$USER` | value from the environment, or `""` if unset |
| `$?` | `shell->last_status` as a string (e.g. `"0"`, `"127"`) |
| `$` at end, `$5`, `$ ` | literal `$` (not a valid name) |

A variable name is `[A-Za-z_][A-Za-z0-9_]*`, exactly what `var_name_len` scans.
`expand_named_var` reads the name, calls [`env_get`](../env/README.md), and
returns a *copy* of the value (the caller owns everything it joins).

---

## The files

| File | What's in it |
|---|---|
| `expander.c` | `expand_cmds` → walks every command, expanding each `argv` entry and each redirection target in place |
| `expander_word.c` | `expand_word` and the per-segment helpers (`expand_single_quoted`, `expand_double_quoted`, `scan_literal`) |
| `expander_var.c` | `expand_var`: resolves `$NAME` and `$?` |

`expand_word` is shared: the [heredoc stage](../heredoc/README.md) reuses
`expand_var` too, but keeps its own line-expander because heredoc bodies must
*not* have quotes removed (bash keeps quotes literal inside a heredoc).

---

## Memory

`expand_word` returns a newly allocated string. Its caller in `expander.c` frees
the original word and stores the new one in its slot:

```c
expanded = expand_word(cmd->argv[i], shell);
free(cmd->argv[i]);
cmd->argv[i] = expanded;
```

If any allocation fails, `expand_word` returns `NULL`, the helpers return `0`,
and `expand_cmds` returns `-1`, so `process_line` skips execution for that
line. Nothing is double-freed because the swap only happens on success.

**Next stage:** the fully expanded command list goes to the
[executor](../executor/README.md).
