/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirs.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 16:29:41 by Visual            #+#    #+#             */
/*   Updated: 2026/07/25 03:01:04 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	open_in(char *target)
{
	int	fd;

	fd = open(target, O_RDONLY);
	if (fd < 0)
		return (ft_putstr_fd("minishell: ", 2), perror(target), -1);
	dup2(fd, STDIN_FILENO);
	close(fd);
	return (0);
}

static int	open_out(char *target, int flags)
{
	int	fd;

	fd = open(target, flags, 0644);
	if (fd < 0)
		return (ft_putstr_fd("minishell: ", 2), perror(target), -1);
	dup2(fd, STDOUT_FILENO);
	close(fd);
	return (0);
}

static int	open_heredoc(t_redir *redir)
{
	if (redir->heredoc_fd < 0)
		return (-1);
	dup2(redir->heredoc_fd, STDIN_FILENO);
	close(redir->heredoc_fd);
	return (0);
}

int	apply_redirs(t_redir *redirs)
{
	int	ret;

	while (redirs)
	{
		if (redirs->type == REDIR_IN)
			ret = open_in(redirs->target);
		else if (redirs->type == REDIR_OUT)
			ret = open_out(redirs->target, O_WRONLY | O_CREAT | O_TRUNC);
		else if (redirs->type == REDIR_APPEND)
			ret = open_out(redirs->target, O_WRONLY | O_CREAT | O_APPEND);
		else
			ret = open_heredoc(redirs);
		if (ret < 0)
			return (-1);
		redirs = redirs->next;
	}
	return (0);
}
