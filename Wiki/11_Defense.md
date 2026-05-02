# 11 — Defense Prep

The questions you must answer cold. Practice saying these out loud.

## Architecture

**Q: Walk me through what happens when I type `ls -l | grep foo > out`.**
1. `readline` returns the line.
2. Lexer produces: WORD(ls), WORD(-l), PIPE, WORD(grep), WORD(foo), REDIR_OUT, WORD(out).
3. Parser builds two `t_cmd`: `[ls, -l]` with no redirs, `[grep, foo]` with one R_OUT redir to `out`.
4. Expander walks each WORD — nothing to expand here, no quotes to strip, words unchanged.
5. Executor sees `n_cmds=2`, creates 1 pipe.
6. Forks child A: dup2 pipe[1] → stdout, exec ls.
7. Forks child B: dup2 pipe[0] → stdin, opens `out` (O_WRONLY|O_CREAT|O_TRUNC), dup2 → stdout, execs grep.
8. Parent closes both pipe ends, waitpid both children, captures grep's exit status into `last_status`.

**Q: Why a flat command table instead of an AST?**
No `&&`/`||`/`()` in mandatory. Pipelines are linear. AST adds complexity I don't need.

**Q: What's in your global variable?**
A `volatile sig_atomic_t g_signal`. Just the signal number. Nothing else.

## Memory & FDs

**Q: Show me one execution and prove no fd leak.**
Run `valgrind --track-fds=yes`. After clean exit, only fds 0/1/2 remain (plus readline's tty fd if applicable).

**Q: How do you free everything on `exit`?**
`clean_and_exit()`: free `envp` slots, free pending `cmds` array, free pending tokens, `rl_clear_history()`, then `exit()`.

**Q: What if malloc fails mid-line?**
Fail the line: free what's been allocated, set `last_status = 1`, write to stderr, return to prompt. Don't crash.

## Quotes & expansion

**Q: Why doesn't `'$USER'` expand?**
Inside single quotes, `$` is literal. The expander tracks quote state and skips `$` handling in SQ.

**Q: Show me `echo "a   b"` — why one argument?**
Inside double quotes, spaces are literal. The lexer doesn't break the WORD on spaces while quote state ≠ NONE.

**Q: Where do quote chars go?**
Stripped by the expander after expansion, before exec. argv has no quote chars in it.

**Q: Does `$VAR` get word-split if its value has spaces?**
Not in my minishell (mandatory doesn't require it). It stays as one argv element. Bash splits it; I don't, deliberately.

## Pipes & exec

**Q: What's the order of pipe operations?**
Pre-create all pipes, fork all children, each child dup2s its end and closes others, parent closes all then waits.

**Q: Why must the parent close pipe fds before waitpid?**
Otherwise the read end stays open in parent and child blocks on read forever.

**Q: How do you find an executable?**
If argv[0] contains `/`, use it directly (check access). Else split `PATH` on `:`, try each dir, take the first with X_OK.

**Q: What's `$?` when no command was ever run?**
0 (initialized).

**Q: After `false | true`, what's `$?`?**
0. Last command in the pipeline wins.

## Signals

**Q: Walk me through Ctrl-C while `cat` is running.**
Parent has SIGINT ignored before forking. Child got SIG_DFL. Kernel sends SIGINT to the foreground process group → cat dies. Parent's waitpid returns. WTERMSIG = SIGINT → status = 128 + 2 = 130. I write a "\n" so the next prompt isn't on cat's line.

**Q: Why ignore SIGINT in the parent during exec?**
Otherwise Ctrl-C kills the shell itself instead of the child.

**Q: Why is `g_signal` `volatile sig_atomic_t`?**
Atomicity: the only type guaranteed safe to access from a signal handler. Volatile: prevent the compiler from caching the read in the main loop.

## Heredoc

**Q: When does the heredoc body get read?**
Before forking the pipeline. The parent reads it via `readline("> ")`, writes to a pipe (or temp file), gives that fd to the child as stdin.

**Q: Does `<< 'EOF'` expand?**
No. The quote on the delimiter disables expansion of the body.

**Q: Ctrl-C during heredoc?**
Custom handler closes stdin so readline returns NULL. After return, I check `g_signal`, free everything, set status = 130, skip the command.

## Builtins

**Q: Why must `cd` run in the parent?**
Because `chdir` only affects the calling process. If forked, the parent's cwd doesn't change.

**Q: But `cd a | echo b` — does cd actually cd?**
No. It runs in a child because pipelines fork everything. Bash does the same.

**Q: What does `exit 1 2` do?**
Prints `exit\n` to stderr (interactive), then "too many arguments", returns 1, **does not exit**. Match bash.

**Q: Edge: `exit "9999999999999999999"`?**
Overflow check: bash treats unparseable as numeric error → exit 2.

## Norm & code quality

**Q: How do you stay under 25-line functions for the executor?**
Split aggressively: `run_pipeline`, `setup_pipes`, `fork_child`, `child_dup`, `parent_wait` — each its own function.

**Q: How do you handle 5-var max in long functions?**
Pack related state into a struct (`t_pipe_ctx { int **pipes; pid_t *pids; int n; }`) when needed.

## What you'd do differently

Be ready: "If you redid this, what would change?"
Honest answers (pick one):
- Adopt a token-fragment model with explicit per-fragment quote state — would simplify expansion.
- Centralize all error-and-exit-code mapping in one table.
- Move heredoc into a forked helper so the main process never blocks on readline during signal storms.

Showing self-awareness > pretending it's perfect.
