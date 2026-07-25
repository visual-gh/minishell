/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 22:01:28 by Visual            #+#    #+#             */
/*   Updated: 2026/07/25 02:10:15 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <signal.h>
# include <errno.h>
# include <limits.h>
# include <stdint.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "../libft/libft.h"

/* ========================================================================== */
/*  GLOBAL                                                                    */
/* ========================================================================== */

extern volatile sig_atomic_t	g_signal;

/* ========================================================================== */
/*  TYPES                                                                     */
/* ========================================================================== */

// WORD			= ls cat hello etc...
// PIPE			= |
// REDIR_IN		= <
// REDIR_OUT	= >
// APPEND		= >>
// HEREDOC		= <<

typedef enum e_tok_type
{
	TOK_WORD,
	TOK_PIPE,
	TOK_REDIR_IN,
	TOK_REDIR_OUT,
	TOK_APPEND,
	TOK_HEREDOC,
}	t_tok_type;

typedef struct s_token
{
	t_tok_type		type;
	char			*value;
	struct s_token	*next;
}	t_token;

typedef enum e_redir_type
{
	REDIR_IN,
	REDIR_OUT,
	REDIR_APPEND,
	REDIR_HEREDOC,
}	t_redir_type;

typedef struct s_redir
{
	t_redir_type	type;
	char			*target;
	int				quoted;
	int				heredoc_fd;
	struct s_redir	*next;
}	t_redir;

typedef struct s_cmd
{
	char			**argv;
	t_redir			*redirs;
	struct s_cmd	*next;
}	t_cmd;

typedef struct s_shell
{
	char			**envp;
	t_cmd			*cmds;
	int				last_status;
}	t_shell;

/* ========================================================================== */
/*  MAIN / SHELL LIFECYCLE                                                    */
/* ========================================================================== */

t_shell	*shell_init(char **envp);
void	shell_loop(t_shell *shell);
void	shell_free(t_shell *shell);

/* ========================================================================== */
/*  SIGNALS                                                                   */
/* ========================================================================== */

void	signals_prompt(void);
void	signals_child(void);
void	signals_heredoc(void);
void	signals_wait(void);

/* ========================================================================== */
/*  ENV                                                                       */
/* ========================================================================== */

char	**env_init(char **envp);
int		env_index(char **envp, const char *key);
char	*env_get(char **envp, const char *key);
int		env_set(char ***envp, const char *key, const char *val);
void	env_unset(char ***envp, const char *key);

/* ========================================================================== */
/*  PIPELINE — LEXER                                                          */
/* ========================================================================== */

t_token	*token_init(t_tok_type type, char *value);
void	add_token(t_token **head, t_token **tail, t_token *new_node);
int		lex_pipe(t_token **head, t_token **tail, int *i);
int		lex_redir(t_token **head, t_token **tail, int *i, int sep);
int		lex_word(t_token **head, t_token **tail, int *i, char *input);
int		separator_type(char *str, int i);
t_token	*lexer(char *input);
void	free_tokens(t_token *head);

/* ========================================================================== */
/*  PIPELINE — PARSER                                                         */
/* ========================================================================== */

int		parse(t_token *tokens, t_shell *shell);
t_cmd	*cmd_new(void);
void	add_cmd(t_cmd **head, t_cmd **tail, t_cmd *new_cmd);
int		count_all_words(t_token *tok);
int		add_redir(t_cmd *cmd, t_redir_type type, char *target, int quoted);
char	*strip_quotes(char *raw, int *quoted);

/* ========================================================================== */
/*  PIPELINE — HEREDOC                                                        */
/* ========================================================================== */

int		read_heredocs(t_shell *shell);
char	*expand_heredoc_line(char *line, t_shell *shell);

/* ========================================================================== */
/*  PIPELINE — EXPANDER                                                       */
/* ========================================================================== */

int		expand_cmds(t_shell *shell);
char	*expand_word(char *word, t_shell *shell);
char	*expand_var(char *str, int *i, t_shell *shell);

/* ========================================================================== */
/*  PIPELINE — EXECUTOR                                                       */
/* ========================================================================== */

int		execute(t_shell *shell);
int		apply_redirs(t_redir *redirs);
char	*resolve_path(char *cmd, char **envp);
int		is_builtin(char *name);
int		run_builtin(t_cmd *cmd, t_shell *shell);
int		exit_code_from(int wstatus);
int		run_single(t_shell *shell, t_cmd *cmd);
int		count_cmds(t_cmd *cmds);
int		**alloc_pipes(int n);
void	close_pipes(int **pipes);
void	run_child(t_shell *shell, t_cmd *cmd);
int		run_pipeline(t_shell *shell);

/* ========================================================================== */
/*  BUILTINS                                                                  */
/* ========================================================================== */

int		ft_echo(t_cmd *cmd);
int		ft_cd(t_cmd *cmd, t_shell *shell);
int		ft_pwd(void);
int		ft_export(t_cmd *cmd, t_shell *shell);
int		ft_unset(t_cmd *cmd, t_shell *shell);
int		ft_env(t_shell *shell);
int		ft_exit(t_cmd *cmd, t_shell *shell);
void	print_export(char **envp);

/* ========================================================================== */
/*  ERRORS / FREE                                                             */
/* ========================================================================== */

int		print_error(char *cmd, char *arg, char *msg);
void	free_cmd_list(t_cmd *cmds);
void	free_redirs(t_redir *redirs);
void	free_str_array(char **arr);

#endif
