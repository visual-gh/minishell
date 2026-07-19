/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   single.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 17:12:09 by Visual            #+#    #+#             */
/*   Updated: 2026/07/19 18:25:07 by Visual           ###   ########.fr       */
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

static void	exec_child(t_shell *shell, t_cmd *cmd)
{
	char	*path;

	signals_child();
	apply_redirs(cmd->redirs);
	if (is_builtin(cmd->argv[0]))
		exit(run_builtin(cmd, shell));
	path = resolve_path(cmd->argv[0], shell->envp);
	if (!path)
	{
		print_error(cmd->argv[0], NULL, "command not found");
		exit(127);
	}
	execve(path, cmd->argv, shell->envp);
	perror(path);
	exit(126);
}

static int	fork_exec(t_shell *shell, t_cmd *cmd)
{
	pid_t	pid;
	int		wstatus;

	pid = fork();
	if (pid == 0)
		exec_child(shell, cmd);
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
