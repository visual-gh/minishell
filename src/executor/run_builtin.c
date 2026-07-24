/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_builtin.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 15:42:51 by Visual            #+#    #+#             */
/*   Updated: 2026/07/24 23:32:05 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	run_builtin(t_cmd *cmd, t_shell *shell)
{
	char	*name;

	name = cmd->argv[0];
	if (!ft_strncmp(name, "cd", SIZE_MAX))
		return (ft_cd(cmd, shell));
	if (!ft_strncmp(name, "echo", SIZE_MAX))
		return (ft_echo(cmd));
	if (!ft_strncmp(name, "env", SIZE_MAX))
		return (ft_env(shell));
	if (!ft_strncmp(name, "exit", SIZE_MAX))
		return (ft_exit(cmd, shell));
	if (!ft_strncmp(name, "export", SIZE_MAX))
		return (ft_export(cmd, shell));
	if (!ft_strncmp(name, "pwd", SIZE_MAX))
		return (ft_pwd());
	if (!ft_strncmp(name, "unset", SIZE_MAX))
		return (ft_unset(cmd, shell));
	return (1);
}
