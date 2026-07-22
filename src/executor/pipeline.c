/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipeline.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 18:42:29 by Visual            #+#    #+#             */
/*   Updated: 2026/07/22 19:54:47 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	child_body(t_shell *shell, t_cmd *cmd, int **pipes, int i)
{
	if (i > 0)
		dup2(pipes[i - 1][0], STDIN_FILENO);
	if (cmd->next)
		dup2(pipes[i][1], STDOUT_FILENO);
	close_pipes(pipes);
	run_child(shell, cmd);
}

static pid_t	*spawn_children(t_shell *shell, int **pipes, int total)
{
	pid_t	*pids;
	t_cmd	*cmd;
	int		i;

	pids = malloc(sizeof(pid_t) * total);
	if (!pids)
		return (NULL);
	cmd = shell->cmds;
	i = 0;
	while (cmd)
	{
		pids[i] = fork();
		if (pids[i] < 0)
			return (print_error(NULL, NULL, "fork failed"), free(pids), NULL);
		if (pids[i] == 0)
			child_body(shell, cmd, pipes, i);
		cmd = cmd->next;
		i++;
	}
	return (pids);
}

static int	wait_all(pid_t *pids, int total)
{
	int	i;
	int	wstatus;
	int	status;

	signals_wait();
	i = 0;
	status = 0;
	while (i < total)
	{
		waitpid(pids[i], &wstatus, 0);
		if (i == total - 1)
			status = exit_code_from(wstatus);
		i++;
	}
	signals_prompt();
	return (status);
}

int	run_pipeline(t_shell *shell)
{
	int		**pipes;
	pid_t	*pids;
	int		total;
	int		status;

	total = count_cmds(shell->cmds);
	pipes = alloc_pipes(total - 1);
	if (!pipes)
		return (print_error(NULL, NULL, "pipe failed"), 1);
	pids = spawn_children(shell, pipes, total);
	if (!pids)
		return (close_pipes(pipes), 1);
	close_pipes(pipes);
	status = wait_all(pids, total);
	free(pids);
	return (status);
}
