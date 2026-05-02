# 10 — Edge Cases & Gotchas

The list every minishell defense circles back to. Test each one against bash, then against yours.

## Empty / whitespace inputs

| Input | Expected |
|---|---|
| (empty line) | new prompt, `$?` unchanged |
| `   ` (whitespace) | same |
| `   \t  ` | same |

Lexer should produce zero tokens → main loop just re-prompts.

## Quote weirdness

| Input | argv | $? |
|---|---|---|
| `echo ""` | `["echo", ""]` → prints empty line | 0 |
| `echo "$"` | prints `$` | 0 |
| `echo '$USER'` | prints `$USER` | 0 |
| `echo "$USER"` | prints value | 0 |
| `echo "'"` | prints `'` | 0 |
| `echo a"b"c'd'e` | prints `abcde` (one argv) | 0 |
| `""` (alone) | `["", ...]` → command not found `` `': command not found `` | 127 |

## Redirection corner cases

| Input | Behavior |
|---|---|
| `> file` | creates/truncates file, no command runs, $?=0 |
| `< nope` | `nope: No such file or directory`, $?=1 |
| `cat < a < b` | opens both, but stdin is the LAST one |
| `cat > a > b` | both files truncated; stdout = b |
| `>` (alone) | syntax error |
| `<<EOF` then immediate Ctrl-D | warning, runs cat with empty input |
| `cat << "EOF"` | body literal, no $ expansion |
| `cat << E$X` (X=OF) | delim becomes `EOF` (expansion happens? **NO**) — in bash, the delim itself is NOT expanded. Tokenize `E$X` as the literal delimiter. |

## Pipes

| Input | $? |
|---|---|
| `false \| true` | 0 (last cmd wins) |
| `true \| false` | 1 |
| `cat \| cat \| ls` | ls's status |
| `nonexistent \| echo hi` | echo runs, prints hi, status of echo (0) |
| `\|` | syntax error |
| `ls \|` | syntax error |

## Variables

| Input | Expansion |
|---|---|
| `echo $USER` | value |
| `echo $usEr` (case sensitive) | value of `usEr` (likely empty) |
| `echo $1` | empty |
| `echo $?` | last status |
| `echo $$` | empty (not supported) or literal `$$` |
| `echo $` | literal `$` |
| `echo "$"` | literal `$` |
| `echo "$USER"x` | `<value>x` |
| `echo $USER:$HOME` | `<usr>:<home>` |
| `echo "$USER""$HOME"` | concat, one argv |

## cd / pwd

| Input | Behavior |
|---|---|
| `cd /tmp && pwd` | `/tmp` (mandatory: no `&&` — but `cd /tmp; pwd` not supported either; just `cd /tmp` then `pwd`) |
| `cd nonexistent` | `cd: nonexistent: No such file or directory`, $?=1 |
| `cd` (no arg) | go to `$HOME` |
| `cd ""` | error: no such file or directory (bash) |
| `cd /` then `pwd` | `/` |

## Path resolution

| Input | $? |
|---|---|
| `nonsense` | `command not found`, 127 |
| `./script.sh` (no x) | `Permission denied`, 126 |
| `/etc` | `Is a directory`, 126 |
| `/bin/ls` | runs, ls's exit status |

## Builtin edge cases

| Input | Notes |
|---|---|
| `echo -nnn hello` | -nnn = flag, prints `hello` no newline |
| `echo -n -n hello` | same |
| `echo -n` | prints nothing, no newline |
| `export =foo` | invalid identifier |
| `export 1abc=x` | invalid identifier, $?=1 |
| `unset PATH; ls` | command not found (no path) |
| `exit 42` | exits with 42 |
| `exit abc` | numeric required, exits 2 |
| `exit 1 2` | "too many arguments", $?=1, **doesn't exit** |
| `exit 256` | exits 0 (256 % 256) |
| `exit -1` | exits 255 |

## Signal interactions

- `cat` then Ctrl-D twice — first sends EOF to cat (cat exits 0), second hits prompt and exits shell.
- `cat \| cat \| cat` then Ctrl-C — all three die, prompt back, $?=130.
- During heredoc, Ctrl-C aborts, $?=130.

## Memory tests to run

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes \
         --suppressions=minishell.supp --error-exitcode=42 \
         ./minishell <<< 'ls -la | wc -l'
```

Test with these inputs in a row, in one valgrind session:
1. `pwd`
2. `echo hello world`
3. `echo "$USER" | cat`
4. `ls -la | grep . | wc -l`
5. `cat << EOF\nhi\nEOF`
6. `nonexistent_cmd`
7. `> /tmp/x; cat < /tmp/x`
8. `exit`

All 8 in one session → 0 bytes lost (post-suppression), no fd leaks.

## Reference behaviors to memorize

- `$?` is **the last foreground pipeline's** exit code.
- Exit codes: 0 OK, 1 generic, 2 syntax, 126 not exec, 127 not found, 128+N signal.
- Builtins in pipes run in subshell — state changes don't stick.
- Quote chars are stripped AFTER expansion, per WORD.
- Heredoc delimiter is NOT expanded; body is (unless delim was quoted).
- `cd` with no arg → `$HOME`; with too many args → error.
