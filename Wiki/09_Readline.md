# 09 — Readline

The library that gives you line editing, history, and the prompt loop. Comes with quirks.

## Functions you're allowed

| Function | Use |
|---|---|
| `readline(const char *prompt)` | print prompt, read a line. Returns malloc'd string or NULL on EOF. |
| `add_history(const char *)` | push line into history (↑ recall) |
| `rl_clear_history()` | wipe all history before exit |
| `rl_on_new_line()` | tells readline cursor is on a new line — call from SIGINT handler |
| `rl_replace_line("", 0)` | clears the current input buffer |
| `rl_redisplay()` | repaints prompt |

## REPL skeleton

```c
char *line;
while (1)
{
    line = readline("minishell$ ");
    if (!line) { ft_putendl_fd("exit", 1); break; }   // Ctrl-D
    if (*line)            // skip empty lines from history
        add_history(line);
    process_line(&sh, line);
    free(line);
}
rl_clear_history();
```

## Compile/link

Ubuntu:
```bash
sudo apt install libreadline-dev
```
Makefile:
```make
LDFLAGS = -lreadline
$(NAME): $(OBJS) $(LIBFT)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) $(LDFLAGS) -o $(NAME)
```

On macOS at 42 schools, readline is in Homebrew — need `-I` and `-L` paths. Not your concern on Ubuntu.

## Memory leak policy

> The readline() function may cause memory leaks, but you are not required to fix them. However, this does not mean your own code can have memory leaks.

Translation: valgrind will scream about readline-internal blocks. Suppress them, don't fix them. Run with a suppression file:

```
# minishell.supp
{
   readline_init
   Memcheck:Leak
   ...
   fun:readline
}
```

```bash
valgrind --leak-check=full --suppressions=minishell.supp ./minishell
```

Or simpler: filter mentally. What matters is that **your** allocations all get freed.

To be sure your code is clean, run with `--leak-check=full --show-leak-kinds=all` and check that every "definitely lost" / "indirectly lost" entry has a stack trace pointing into readline. Anything pointing into your code = bug.

## Non-interactive input

`./minishell < script.sh` — readline still works, but no prompt is shown when stdin isn't a TTY. Detect:

```c
if (isatty(STDIN_FILENO))
    line = readline("minishell$ ");
else
    line = readline(NULL);
```

Subject doesn't require non-interactive mode but graders sometimes test it. Don't over-engineer — making it not crash is enough.

## Common pitfalls

- **Forgetting to `free(line)`** every iteration → leak that's yours, not readline's.
- **Calling `add_history(line)` when line is empty/whitespace** → pollutes history. Skip empty.
- **Calling `rl_*` from outside a SIGINT handler** → may corrupt state. Only inside the handler.
- **Mixing readline with `printf` for the prompt** — readline owns terminal state; build the prompt string and pass it to readline.
- **Linking without `-lreadline`** → undefined symbol on every readline call. Norm-clean code, dead binary.
