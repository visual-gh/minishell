# src/: How minishell is built

This is the map. If you are learning the project from zero, start here, then
follow the reading order at the bottom. Every subdirectory has its own README
that goes deep on one stage; this one explains how they fit together.

---

## The one idea behind the whole project

A shell does one thing in a loop: **read a line of text, turn it into something
runnable, run it.** That's it. `shell_loop` is literally that loop:

```c
while (1)
{
    line = readline("minishell$ ");   // read text
    ...
    process_line(line, shell);        // turn it into structs, run them
    free(line);
}
```

The hard part is the middle: going from a flat string like
`echo "hi $USER" | wc -c` to actual processes with their input and output wired
together. We don't do it in one leap. We do it in **five small stages**, each of
which takes a clean structure and hands the next stage a slightly richer one.

---

## The pipeline

```
  "echo \"hi $USER\" | wc -c"          one raw string
            │
   ┌────────▼─────────┐
   │ 1. LEXER         │  string  ──►  token list        (words and operators)
   └────────┬─────────┘
   ┌────────▼─────────┐
   │ 2. PARSER        │  tokens  ──►  command list       (grouped by pipes,
   └────────┬─────────┘                                   redirs attached)
   ┌────────▼─────────┐
   │ 3. HEREDOC       │  reads << bodies into pipes before anything runs
   └────────┬─────────┘
   ┌────────▼─────────┐
   │ 4. EXPANDER      │  rewrites $VAR / $? and removes quotes, in place
   └────────┬─────────┘
   ┌────────▼─────────┐
   │ 5. EXECUTOR      │  forks, wires pipes and redirs, runs the commands
   └────────┬─────────┘
            ▼
      exit status  ──►  $?
```

`process_line` is the whole pipeline in eight lines:

```c
tokens = lexer(line);                       // stage 1
if (parse(tokens, shell) == -1)             // stage 2
    shell->last_status = 2;
else if (ready_to_execute(shell))           // stages 3 + 4
    shell->last_status = execute(shell);    // stage 5
```

where `ready_to_execute` is just:

```c
return (shell->cmds && read_heredocs(shell) == 0 && expand_cmds(shell) == 0);
```

The `&&` chain is deliberate: if there are no commands, or a heredoc was
interrupted with Ctrl-C, or an expansion failed, we stop before executing.

---

## The data, as it transforms

Each stage has a type it produces. Learn these four structs (all defined and
explained in [includes/](../includes/README.md)) and you understand the data flow:

| Stage | Produces | Shape |
|---|---|---|
| Lexer | `t_token` list | `WORD("echo") → WORD("\"hi $USER\"") → PIPE → WORD("wc") → WORD("-c")` |
| Parser | `t_cmd` list | `cmd{argv:[echo, "hi $USER"]} → cmd{argv:[wc, -c]}` |
| Expander | mutates `t_cmd` | `argv` becomes `[echo, hi visual]`: quotes gone, `$USER` resolved |
| Executor | an exit code | forks two children, pipes stdout→stdin, returns `wc`'s status |

Follow one command all the way through and it clicks:

```
input     echo "hi $USER" | wc -c

lexer     [WORD echo] [WORD "hi $USER"] [PIPE] [WORD wc] [WORD -c]

parser    cmd A: argv = { "echo", "\"hi $USER\"" }, redirs = none
          cmd B: argv = { "wc", "-c" },             redirs = none

expander  cmd A: argv = { "echo", "hi visual" }
          cmd B: argv = { "wc", "-c" }

executor  fork A, fork B, connect A's stdout to B's stdin via a pipe,
          wait for both, return B's exit code
          → prints "9"
```

---

## The directory map

Read-eval loop and setup live directly in `src/`:

- **`main.c`** validates argc, builds the shell, runs the loop, returns `$?`.
- **`shell_init.c`** allocates `t_shell`, copies the environment, installs signal handlers.
- **`shell_loop.c`** is the read-eval loop and `process_line` (the pipeline above).

Each stage and support layer is its own module:

| Module | Role | README |
|---|---|---|
| [`lexer/`](lexer/README.md) | raw line → token list; flags unclosed quotes | tokenizing |
| [`parser/`](parser/README.md) | token list → command list; syntax errors | grouping |
| [`expander/`](expander/README.md) | `$VAR` / `$?` and quote removal | the single owner of quoting |
| [`heredoc/`](heredoc/README.md) | reads `<<` bodies before execution | the tricky pre-step |
| [`executor/`](executor/README.md) | forks, pipes, redirects, dispatches builtins | the engine |
| [`builtins/`](builtins/README.md) | `echo cd pwd export unset env exit` | the seven built-ins |
| [`env/`](env/README.md) | the `char **` environment and its four operations | state |
| [`signals/`](signals/README.md) | Ctrl-C / Ctrl-\ / Ctrl-D behaviour | the one global |
| [`utils/`](utils/README.md) | error printing and the `free_*` family | plumbing |

---

## Who owns memory

One rule keeps the whole thing leak-free: **each stage frees the structure it
consumes, and `t_shell` owns everything still live.**

```
lexer      tokens  ──►  freed by process_line right after parse (free_tokens)
parser     cmds    ──►  held on shell->cmds, freed after each line (free_cmd_list)
expander   mutates cmds in place: frees the old string, stores the new one
executor   forks: children exit (their copies vanish), parent frees nothing new
shutdown   shell_free: frees cmds, then the env array, then the struct
```

The only leaks tolerated are the ones *inside* `readline` itself, which the
subject explicitly exempts. Everything we allocate, we free. Verify with:

```bash
printf 'ls | wc -l\nexit\n' | valgrind --leak-check=full ./minishell
```

---

## Suggested reading order (learning from zero)

1. **[includes/](../includes/README.md)**: the four structs. Nothing else makes
   sense until you can picture a `t_cmd`.
2. **[lexer/](lexer/README.md)** then **[parser/](parser/README.md)**: how text
   becomes those structs. Read them together.
3. **[expander/](expander/README.md)**: the quoting rules, the cleverest part.
4. **[executor/](executor/README.md)**: fork/pipe/dup2, the heart of the shell.
   Take your time here.
5. **[heredoc/](heredoc/README.md)**, **[signals/](signals/README.md)**: the two
   pieces that trip people up in defense. Small but subtle.
6. **[builtins/](builtins/README.md)**, **[env/](env/README.md)**,
   **[utils/](utils/README.md)**: the supporting cast; quick reads.

For the big picture of the whole project (build, features, testing), see the
[root README](../README.md).
