/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 09:41:52 by Daniela           #+#    #+#             */
/*   Updated: 2026/07/22 16:25:27 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static t_redir	*redir_new(t_redir_type type, char *target, int quoted)
{
	t_redir	*redir;

	redir = malloc(sizeof(t_redir));
	if (redir == NULL)
		return (NULL);
	redir->type = type;
	redir->target = ft_strdup(target);
	if (redir->target == NULL)
	{
		free(redir);
		return (NULL);
	}
	redir->quoted = quoted;
	redir->heredoc_fd = -1;
	redir->next = NULL;
	return (redir);
}

int	add_redir(t_cmd *cmd, t_redir_type type, char *target, int quoted)
{
	t_redir	*redir;
	t_redir	*current;

	redir = redir_new(type, target, quoted);
	if (redir == NULL)
		return (-1);
	if (cmd->redirs == NULL)
		cmd->redirs = redir;
	else
	{
		current = cmd->redirs;
		while (current->next != NULL)
			current = current->next;
		current->next = redir;
	}
	return (0);
}
