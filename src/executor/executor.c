/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 15:41:28 by Visual            #+#    #+#             */
/*   Updated: 2026/07/25 01:58:01 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	execute(t_shell *shell)
{
	if (count_cmds(shell->cmds) == 1)
		return (run_single(shell, shell->cmds));
	return (run_pipeline(shell));
}

int	exit_code_from(int wstatus)
{
	if (WIFEXITED(wstatus))
		return (WEXITSTATUS(wstatus));
	if (WIFSIGNALED(wstatus))
		return (128 + WTERMSIG(wstatus));
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
