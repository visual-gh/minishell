/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/17 18:51:20 by Visual            #+#    #+#             */
/*   Updated: 2026/07/25 03:09:59 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*get_target(t_cmd *cmd, t_shell *shell)
{
	char	*home;

	if (cmd->argv[1] == NULL)
	{
		home = env_get(shell->envp, "HOME");
		if (home == NULL)
			print_error("cd", NULL, "HOME not set");
		return (home);
	}
	return (cmd->argv[1]);
}

int	ft_cd(t_cmd *cmd, t_shell *shell)
{
	char	*target;
	char	old_cwd[PATH_MAX];
	char	new_cwd[PATH_MAX];

	if (cmd->argv[1] && cmd->argv[2])
	{
		print_error("cd", NULL, "too many arguments");
		return (1);
	}
	target = get_target(cmd, shell);
	if (target == NULL)
		return (1);
	if (getcwd(old_cwd, sizeof(old_cwd)) == NULL || chdir(target) == -1)
		return (print_error("cd", target, strerror(errno)), 1);
	env_set(&shell->envp, "OLDPWD", old_cwd);
	if (getcwd(new_cwd, sizeof(new_cwd)) != NULL)
		env_set(&shell->envp, "PWD", new_cwd);
	return (0);
}
