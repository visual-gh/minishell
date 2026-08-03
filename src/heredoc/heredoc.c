/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 17:12:32 by Visual            #+#    #+#             */
/*   Updated: 2026/08/03 17:01:17 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	write_heredoc_line(int fd, char *line, t_redir *redir,
		t_shell *shell)
{
	char	*exp;

	if (!redir->quoted)
		exp = expand_heredoc_line(line, shell);
	else
		exp = ft_strdup(line);
	write(fd, exp, ft_strlen(exp));
	write(fd, "\n", 1);
	free(exp);
}

static int	read_heredoc_body(t_redir *redir, t_shell *shell)
{
	int		pipefd[2];
	char	*line;

	if (pipe(pipefd) < 0)
		return (-1);
	line = readline("> ");
	while (line && !g_signal && ft_strncmp(line, redir->target, SIZE_MAX) != 0)
	{
		write_heredoc_line(pipefd[1], line, redir, shell);
		free(line);
		line = readline("> ");
	}
	free(line);
	close(pipefd[1]);
	if (g_signal)
		return (close(pipefd[0]), -1);
	redir->heredoc_fd = pipefd[0];
	return (0);
}

static int	process_cmd_heredocs(t_cmd *cmd, t_shell *shell)
{
	t_redir	*redir;

	redir = cmd->redirs;
	while (redir)
	{
		if (redir->type == REDIR_HEREDOC
			&& read_heredoc_body(redir, shell) < 0)
			return (-1);
		redir = redir->next;
	}
	return (0);
}

int	read_heredocs(t_shell *shell)
{
	t_cmd	*cmd;
	int		saved_stdin;
	int		ret;

	saved_stdin = dup(STDIN_FILENO);
	signals_heredoc();
	cmd = shell->cmds;
	ret = 0;
	while (cmd && ret == 0)
	{
		ret = process_cmd_heredocs(cmd, shell);
		cmd = cmd->next;
	}
	dup2(saved_stdin, STDIN_FILENO);
	close(saved_stdin);
	signals_prompt();
	if (g_signal)
	{
		ft_putstr_fd("\n", STDERR_FILENO);
		shell->last_status = 130;
		g_signal = 0;
		ret = -1;
	}
	return (ret);
}
