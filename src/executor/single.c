/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   single.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 17:12:09 by Visual            #+#    #+#             */
/*   Updated: 2026/07/20 02:13:45 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	apply_redirs_only(t_cmd *cmd)
{
	int	saved_in;
	int	saved_out;
	int	status;

	saved_in = dup(STDIN_FILENO);
	saved_out = dup(STDOUT_FILENO);
	if (apply_redirs_ret(cmd->redirs) < 0)
		status = 1;
	else
		status = 0;
	dup2(saved_in, STDIN_FILENO);
	dup2(saved_out, STDOUT_FILENO);
	close(saved_in);
	close(saved_out);
	return (status);
}

static int	run_builtin_in_parent(t_shell *shell, t_cmd *cmd)
{
	int	saved_in;
	int	saved_out;
	int	status;

	saved_in = dup(STDIN_FILENO);
	saved_out = dup(STDOUT_FILENO);
	if (apply_redirs_ret(cmd->redirs) < 0)
		status = 1;
	else
		status = run_builtin(cmd, shell);
	dup2(saved_in, STDIN_FILENO);
	dup2(saved_out, STDOUT_FILENO);
	close(saved_in);
	close(saved_out);
	return (status);
}

static int	fork_exec(t_shell *shell, t_cmd *cmd)
{
	pid_t	pid;
	int		wstatus;

	pid = fork();
	if (pid == 0)
		run_child(shell, cmd);
	signals_wait();
	waitpid(pid, &wstatus, 0);
	signals_prompt();
	return (exit_code_from(wstatus));
}

int	run_single(t_shell *shell, t_cmd *cmd)
{
	if (!cmd->argv || !cmd->argv[0])
		return (apply_redirs_only(cmd));
	if (is_builtin(cmd->argv[0]))
		return (run_builtin_in_parent(shell, cmd));
	return (fork_exec(shell, cmd));
}
