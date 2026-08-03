/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 15:41:28 by Visual            #+#    #+#             */
/*   Updated: 2026/08/03 16:51:40 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	execute(t_shell *shell)
{
	if (count_cmds(shell->cmds) == 1)
		return (run_single(shell, shell->cmds));
	return (run_pipeline(shell));
}

static void	report_signal(int wstatus)
{
	int	sig;

	sig = WTERMSIG(wstatus);
	if (sig == SIGINT)
		ft_putstr_fd("\n", STDERR_FILENO);
	else if (sig == SIGQUIT)
	{
		ft_putstr_fd("Quit", STDERR_FILENO);
		if (WCOREDUMP(wstatus))
			ft_putstr_fd(" (core dumped)", STDERR_FILENO);
		ft_putstr_fd("\n", STDERR_FILENO);
	}
}

int	exit_code_from(int wstatus)
{
	if (WIFEXITED(wstatus))
		return (WEXITSTATUS(wstatus));
	if (WIFSIGNALED(wstatus))
	{
		report_signal(wstatus);
		return (128 + WTERMSIG(wstatus));
	}
	return (1);
}

int	is_builtin(char *name)
{
	if (!name)
		return (0);
	if (!ft_strncmp(name, "cd", SIZE_MAX)
		|| !ft_strncmp(name, "echo", SIZE_MAX)
		|| !ft_strncmp(name, "env", SIZE_MAX)
		|| !ft_strncmp(name, "exit", SIZE_MAX)
		|| !ft_strncmp(name, "export", SIZE_MAX)
		|| !ft_strncmp(name, "pwd", SIZE_MAX)
		|| !ft_strncmp(name, "unset", SIZE_MAX))
		return (1);
	return (0);
}
