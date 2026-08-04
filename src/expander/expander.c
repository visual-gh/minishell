/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 14:43:43 by Visual            #+#    #+#             */
/*   Updated: 2026/08/03 19:53:31 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	expand_redir_list(t_redir *redir, t_shell *shell)
{
	char	*expanded;

	while (redir)
	{
		if (redir->type != REDIR_HEREDOC)
		{
			expanded = expand_word(redir->target, shell);
			if (!expanded)
				return (0);
			free(redir->target);
			redir->target = expanded;
		}
		redir = redir->next;
	}
	return (1);
}

static int	drop_word(char *raw, char *expanded)
{
	if (*expanded)
		return (0);
	if (ft_strchr(raw, '\'') || ft_strchr(raw, '"'))
		return (0);
	return (1);
}

static int	expand_argv(t_cmd *cmd, t_shell *shell)
{
	char	*expanded;
	char	*raw;
	int		i;
	int		j;

	i = 0;
	j = 0;
	while (cmd->argv[i])
	{
		raw = cmd->argv[i];
		expanded = expand_word(raw, shell);
		if (!expanded)
			return (cmd->argv[j] = NULL, 0);
		if (drop_word(raw, expanded))
			free(expanded);
		else
			cmd->argv[j++] = expanded;
		free(raw);
		i++;
	}
	cmd->argv[j] = NULL;
	return (1);
}

static int	expand_cmd(t_cmd *cmd, t_shell *shell)
{
	if (cmd->argv && !expand_argv(cmd, shell))
		return (0);
	if (cmd->redirs && !expand_redir_list(cmd->redirs, shell))
		return (0);
	return (1);
}

int	expand_cmds(t_shell *shell)
{
	t_cmd	*cmd;

	cmd = shell->cmds;
	while (cmd)
	{
		if (!expand_cmd(cmd, shell))
			return (-1);
		cmd = cmd->next;
	}
	return (0);
}
