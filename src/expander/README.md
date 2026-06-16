# src/expander/: Expander

The expander runs between the parser and the executor.
Its job is to take the raw words from the token list and turn them into their final values before any command runs.

Given a command like:

```
echo "Hello $USER, you are in '$PWD'"
```

The parser hands us three words: `echo`, `"Hello $USER, you are in '$PWD'"`, and nothing else.
The expander turns that middle word into `Hello visual, you are in '$PWD'` and passes it to the executor.

---

## The three quote rules

Everything the expander does comes down to three rules:

**Single quotes**: nothing is interpreted. `'$HOME'` stays `$HOME`. The content is copied verbatim.

**Double quotes**: only `$` is interpreted. `"$HOME"` becomes `/home/visual`. Everything else is literal, including single quotes inside.

**No quotes**: `$` is interpreted, same as double quotes.

---

## How it processes a word

A word like `"Hello $USER"` is not a single thing to expand; it is a sequence of segments with different rules.
The expander walks the word character by character and handles each segment based on what it finds:

```
"Hello $USER"
^             starts with "  : enter double-quote mode
 Hello        plain text     : copy as-is            →  "Hello "
       ^      $ found        : look up USER in env   →  "visual"
            ^ " found        : exit double-quote mode
```

Each segment produces a string. They are joined together left to right into the final result.

The same logic applies to mixed words like `"$HOME"/.config/'my app'`:

| Segment   | Rule          | Result             |
|-----------|---------------|--------------------|
| `"$HOME"` | double-quoted | `/home/visual`     |
| `/`       | plain         | `/`                |
| `.config/`| plain         | `.config/`         |
| `'my app'`| single-quoted | `my app`           |
| **result**|               | `/home/visual/.config/my app` |

---

## Variable lookup

When the expander hits a `$`, it checks what follows:

- `$HOME`, `$USER`, `$_VAR`: looks up the name in `shell->envp`. Returns the value, or an empty string if not set.
- `$?`: returns the exit status of the last command as a string.
- `$` at end of word, or `$5`, `$ `: not a valid variable. The `$` is kept as a literal.

---

## The files

**`expander.c`**: entry point. Walks every command in `shell->cmds` and calls `expand_word` on each argument and each redirection target. Replaces the original string with the expanded result in place.

**`expander_word.c`**: implements `expand_word`. Walks a single word and handles the quote logic described above. Uses a helper `strjoin_free` to build up the result string by joining chunks and freeing the intermediate pieces as it goes.

**`expander_var.c`**: implements `expand_var`. Called whenever a `$` is found. Reads the variable name, looks it up with `env_get`, and returns the value as a fresh string.

---

## Memory

`expand_word` returns a newly allocated string. The caller (`expand_argv` or `expand_redir_list`) frees the original word and stores the result in its place. If any allocation fails, the function returns `NULL` and the error propagates up to `expand_cmds`.
