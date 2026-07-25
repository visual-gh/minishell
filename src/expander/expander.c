/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 14:43:43 by Visual            #+#    #+#             */
/*   Updated: 2026/07/25 02:25:53 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	expand_redir_list(t_redir *redir, t_shell *shell)
{
	char	*expanded;

	while (redir)
	{
		expanded = expand_word(redir->target, shell);
		if (!expanded)
			return (0);
		free(redir->target);
		redir->target = expanded;
		redir = redir->next;
	}
	return (1);
}

static int	expand_argv(t_cmd *cmd, t_shell *shell)
{
	char	*expanded;
	int		i;

	i = 0;
	while (cmd->argv[i])
	{
		expanded = expand_word(cmd->argv[i], shell);
		if (!expanded)
			return (0);
		free(cmd->argv[i]);
		cmd->argv[i] = expanded;
		i++;
	}
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
