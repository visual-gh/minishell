# Minishell To Do

Status: phase 0 in progress. Project compiles, exits 0, rejects bad argc.

## Phase 0: Skeleton

- [x] `src/main.c`: argc check, init, loop, free, return last_status
- [x] `src/shell_init.c`: allocate `t_shell`, dup envp, install signals
- [x] `src/shell_loop.c`: readline + add_history loop, Ctrl-D exits
- [x] `src/env/env_init.c`: deep copy envp
- [x] `src/env/env_get.c`: lookup by key
- [x] `src/env/env_set.c`: insert or update key=val
- [x] `src/env/env_unset.c`: remove key, compact array
- [x] `src/signals/signals.c`: `signals_prompt` (SIGINT redraws, SIGQUIT ignored), stubs for child / wait / heredoc
- [x] `src/utils/error.c`: `print_error`
- [x] `src/utils/free.c`: `free_cmd_list`, `free_redirs`, `free_str_array`
- [x] `Makefile`: -Wall -Wextra -Werror, -lreadline, libft auto-build, no relink
- [x] Smoke test: prompt works, Ctrl-C redraws, Ctrl-D exits, valgrind clean

## Phase 1: Lexer

- [ ] `src/lexer/lexer.c`: tokenize loop, returns `t_token *`
- [ ] `src/lexer/lexer_quotes.c`: quote handling, sets `quoted` flag
- [ ] `src/lexer/lexer_utils.c`: operator detection (`|`, `<`, `>`, `<<`, `>>`), append, free
- [ ] Test by printing token list for sample inputs

## Phase 2: Parser

- [ ] `src/parser/parser.c`: token list to `t_cmd` list
- [ ] `src/parser/parser_redir.c`: attach redirs to current cmd
- [ ] `src/parser/parser_syntax.c`: syntax errors (status 2)

## Phase 3: Heredoc

- [ ] `src/heredoc/heredoc.c`: read bodies before fork, expand if delim unquoted, heredoc signal mode

## Phase 4: Expander

- [ ] `src/expander/expander.c`: walk argv and redir targets
- [ ] `src/expander/expander_var.c`: `$VAR`, `$?`, quote-aware, drop empty unquoted words

## Phase 5: Executor

- [ ] `src/executor/exec.c`: single cmd vs pipeline dispatch, builtin in parent for single-builtin lines
- [ ] `src/executor/exec_pipe.c`: pipeline fork, dup2, wait
- [ ] `src/executor/exec_redir.c`: `apply_redirs`
- [ ] `src/executor/exec_path.c`: `resolve_path` via PATH

## Phase 6: Builtins

- [ ] `echo` with `-n`
- [ ] `pwd`
- [ ] `env`
- [ ] `cd` (update PWD and OLDPWD)
- [ ] `export` (no args lists sorted, `=` sets, no `=` marks)
- [ ] `unset`
- [ ] `exit` (numeric arg, too many args)

## Phase 7: Polish

- [ ] Norminette clean on all `.c` and `.h`
- [ ] Valgrind clean with `--leak-check=full --track-fds=yes` on every test case
- [ ] Edge cases: empty line, whitespace only, unclosed quotes, `|` at start or end, `$?`, very long input
- [ ] Defense walkthrough vs `Wiki/11_Defense.md`
