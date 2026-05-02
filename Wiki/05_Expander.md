# 05 — Expander

Walk every `argv[i]` and every redir target, replacing `$VAR` and `$?`, then strip surviving quote characters. Done in-place: free old string, set new.

## What expands

| Trigger | Replacement |
|---|---|
| `$VAR` | value of `VAR` in envp, or `""` if unset |
| `$?` | `last_status` as decimal |
| `$0`, `$1`, ... | `""` (positional params not supported) |
| `$$`, `$!`, `$#` | `""` (subject doesn't ask) — or treat `$$` literally; pick one |
| `$` followed by non-name | literal `$` |

A "name" character: `[a-zA-Z_][a-zA-Z0-9_]*`. Stop at the first non-name char.

## Quote rules (the whole game)

- Inside `'...'` → **no expansion**. `$USER` is literal.
- Inside `"..."` → **expand `$VAR`/`$?`**. Spaces preserved.
- Outside quotes → expand normally.

The lexer kept the quote characters in the WORD's `value`. The expander walks the string and tracks its own quote state as it goes. It expands `$` only when state ≠ SQ.

After expansion, **strip the quote chars** from the final string.

## No word-splitting (mandatory)

In real bash, unquoted `$VAR` containing spaces is split into multiple words and re-globbed. **Don't do this.** Subject doesn't require it; bash splitting is a bonus-tier complication. Treat the expansion as one literal string.

So:
```sh
X="a b c"
echo $X    →  one argv: "a b c"   (in minishell)
            →  three argv (a b c) in bash
```

The peer evaluator will accept the simpler behavior because the subject doesn't list field-splitting as a requirement. If they push back, this is a deliberate, defensible choice — say so.

## Heredoc delimiter quoting

```sh
cat << EOF       # body IS expanded
$USER
EOF

cat << "EOF"     # body NOT expanded — $USER stays literal
$USER
EOF
```

The parser stored `quoted` on the heredoc redir. Expander uses that flag when reading the heredoc body to decide whether to expand each line.

## Algorithm

```c
char *expand_word(char *src, t_shell *sh)
{
    t_strbuf out;            // dynamic buffer
    int      state = NONE;
    int      i = 0;

    while (src[i])
    {
        if (state != DQ && src[i] == '\'')      { toggle SQ; i++; continue; }
        if (state != SQ && src[i] == '"')       { toggle DQ; i++; continue; }
        if (state != SQ && src[i] == '$')       { append_var(&out, src, &i, sh); continue; }
        strbuf_addc(&out, src[i]);
        i++;
    }
    return strbuf_finish(&out);
}
```

`append_var` peeks at `src[i+1]`:
- if `?` → append `last_status`, advance 2
- if name char → read name, lookup, append value (or empty), advance past name
- else → append literal `$`, advance 1

## What "empty after expansion" means

```sh
echo $UNSET hello
```
`$UNSET` expands to `""`. argv stays: `{"echo", "", "hello", NULL}`. echo prints a single space then "hello". Match bash.

If a **whole** unquoted word expands to `""` (e.g., `cat $UNSET`), bash drops it (`cat`). Without word-splitting, you can either:
- Keep the empty argv (defensible, subject doesn't say)
- Drop empty words after expansion when they came from a fully unquoted source

Pick one. The simplest (and what most minishells do) is: **drop empty unquoted words after expansion**, but **keep empty quoted words** (`""` → empty argv element).

## Order of operations

1. Lex
2. Parse (syntax check, build cmds)
3. **For each heredoc**: read body now (with expansion if delim was unquoted)
4. **For each argv element + each redir target**: expand, then strip quotes
5. Execute

Expansion happens *after* parsing but *before* file lookup, PATH search, and exec.

## Quote stripping subtlety

`""$UNSET""` → after expansion = `""""` → after strip = `""` (preserved as empty argv).
`$UNSET` (unquoted) → expansion = `""` → after strip = `""` → drop from argv.

Track a flag `had_quote` while expanding. If true and final string is empty → keep. If false and empty → drop.
