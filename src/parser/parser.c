/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 17:12:54 by Daniela           #+#    #+#             */
/*   Updated: 2026/07/21 20:38:59 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static t_redir_type	redir_type_from_tok(t_tok_type type)
{
	if (type == TOK_REDIR_IN)
		return (REDIR_IN);
	if (type == TOK_REDIR_OUT)
		return (REDIR_OUT);
	if (type == TOK_APPEND)
		return (REDIR_APPEND);
	return (REDIR_HEREDOC);
}

static int	parse_redir(t_cmd *cmd, t_token **tok)
{
	t_redir_type	type;

	type = redir_type_from_tok((*tok)->type);
	*tok = (*tok)->next;
	if (*tok == NULL || (*tok)->type != TOK_WORD)
	{
		print_error(NULL, NULL, "syntax error near unexpected token `newline'");
		return (-1);
	}
	if (add_redir(cmd, type, (*tok)->value, (*tok)->quoted) == -1)
		return (-1);
	*tok = (*tok)->next;
	return (0);
}

static int	parse_words(t_cmd *cmd, t_token **tok)
{
	int	count;

	count = count_args(*tok);
	cmd->argv = make_argv(*tok, count);
	if (cmd->argv == NULL)
		return (-1);
	while (count-- > 0)
	{
		while ((*tok)->join && (*tok)->next)
			*tok = (*tok)->next;
		*tok = (*tok)->next;
	}
	return (0);
}

static int	parse_cmd(t_cmd *cmd, t_token **tok)
{
	while (*tok != NULL && (*tok)->type != TOK_PIPE)
	{
		if ((*tok)->type == TOK_WORD)
		{
			if (parse_words(cmd, tok) == -1)
				return (-1);
		}
		else if (parse_redir(cmd, tok) == -1)
			return (-1);
	}
	if (cmd->argv == NULL && cmd->redirs == NULL)
	{
		print_error(NULL, NULL, "syntax error near unexpected token `|'");
		return (-1);
	}
	return (0);
}

int	parse(t_token *tokens, t_shell *shell)
{
	t_token	*tok;
	t_cmd	*cmd;

	if (tokens == NULL)
		return (0);
	tok = tokens;
	while (1)
	{
		cmd = cmd_new();
		if (cmd == NULL)
			return (-1);
		add_cmd(&shell->cmds, cmd);
		if (parse_cmd(cmd, &tok) == -1)
			return (-1);
		if (tok == NULL)
			break ;
		tok = tok->next;
	}
	return (0);
}
