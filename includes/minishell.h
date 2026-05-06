/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: danielad <danielad@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 22:01:28 by Visual            #+#    #+#             */
/*   Updated: 2026/05/05 21:58:46 by danielad         ###   ########.fr       */
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
# include <string.h>
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

typedef enum e_tok_type
{
	TOK_WORD,    //ls cat hello ..
	TOK_PIPE,     // | 
	TOK_REDIR_IN,  // <
	TOK_REDIR_OUT,  // >
	TOK_APPEND,     // >>
	TOK_HEREDOC,	// <<
}	t_tok_type;

typedef struct s_token
{
	t_tok_type		type;
	char			*value;
	int				quoted;
	int				join;
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
char	*env_get(char **envp, const char *key);
int		env_set(char ***envp, const char *key, const char *val);
void	env_unset(char ***envp, const char *key);
void	env_free(char **envp);

/* ========================================================================== */
/*  PIPELINE — LEXER                                                          */
/* ========================================================================== */

t_token	*lexer(char *input);
void	free_tokens(t_token *tokens);

/* ========================================================================== */
/*  PIPELINE — PARSER                                                         */
/* ========================================================================== */

int		parse(t_token *tokens, t_shell *shell);

/* ========================================================================== */
/*  PIPELINE — HEREDOC                                                        */
/* ========================================================================== */

int		read_heredocs(t_shell *shell);

/* ========================================================================== */
/*  PIPELINE — EXPANDER                                                       */
/* ========================================================================== */

int		expand_cmds(t_shell *shell);

/* ========================================================================== */
/*  PIPELINE — EXECUTOR                                                       */
/* ========================================================================== */

int		execute(t_shell *shell);
int		apply_redirs(t_redir *redirs);
char	*resolve_path(char *cmd, char **envp);
int		is_builtin(char *name);
int		run_builtin(t_cmd *cmd, t_shell *shell);

/* ========================================================================== */
/*  BUILTINS                                                                  */
/* ========================================================================== */

int		builtin_echo(t_cmd *cmd);
int		builtin_cd(t_cmd *cmd, t_shell *shell);
int		builtin_pwd(void);
int		builtin_export(t_cmd *cmd, t_shell *shell);
int		builtin_unset(t_cmd *cmd, t_shell *shell);
int		builtin_env(t_shell *shell);
int		builtin_exit(t_cmd *cmd, t_shell *shell);

/* ========================================================================== */
/*  ERRORS / FREE                                                             */
/* ========================================================================== */

int		print_error(char *cmd, char *arg, char *msg);
void	free_cmd_list(t_cmd *cmds);
void	free_redirs(t_redir *redirs);
void	free_str_array(char **arr);



// token_init functions   will clean after

t_token *token_init(t_tok_type type, char *value, int quoted);
void	add_token(t_token **head, t_token *new_node);





#endif
