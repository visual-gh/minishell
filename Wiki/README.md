# Minishell — Wiki

Full course on the project. Read in order.

## Index

1. [01_Overview.md](01_Overview.md) — What you're building, mental model, scope
2. [02_Architecture.md](02_Architecture.md) — Pipeline (Lexer → Parser → Expander → Executor), data structures
3. [03_Lexer.md](03_Lexer.md) — Tokenization rules, quotes, metacharacters
4. [04_Parser.md](04_Parser.md) — Token stream → flat command table
5. [05_Expander.md](05_Expander.md) — `$VAR`, `$?`, quote-aware expansion
6. [06_Executor.md](06_Executor.md) — fork/exec/pipe/dup2, redirections, heredoc
7. [07_Builtins.md](07_Builtins.md) — echo / cd / pwd / export / unset / env / exit
8. [08_Signals.md](08_Signals.md) — SIGINT, SIGQUIT, EOF, the one global rule
9. [09_Readline.md](09_Readline.md) — readline API, history, leaks policy
10. [10_EdgeCases.md](10_EdgeCases.md) — Bash references, error codes, gotchas
11. [11_Defense.md](11_Defense.md) — Questions you must answer cold

Quick reference at the project root: `Cheatsheet.md`.

## Golden rule

When in doubt → **ask bash**. Run the equivalent in real bash, observe exit code (`echo $?`), copy the behavior. Do not invent.
