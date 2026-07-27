/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_child.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 18:52:16 by Visual            #+#    #+#             */
/*   Updated: 2026/07/27 17:53:22 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	**exported_envp(char **envp)
{
	char	**out;
	int		i;
	int		n;

	n = 0;
	while (envp[n])
		n++;
	out = ft_calloc(n + 1, sizeof(char *));
	if (!out)
		return (envp);
	i = 0;
	n = 0;
	while (envp[i])
	{
		if (ft_strchr(envp[i], '='))
			out[n++] = envp[i];
		i++;
	}
	return (out);
}

void	run_child(t_shell *shell, t_cmd *cmd)
{
	char	*path;
	char	**envp;

	signals_child();
	if (apply_redirs(cmd->redirs) < 0)
		exit(1);
	if (!cmd->argv || !cmd->argv[0])
		exit(0);
	if (is_builtin(cmd->argv[0]))
		exit(run_builtin(cmd, shell));
	path = resolve_path(cmd->argv[0], shell->envp);
	if (!path)
	{
		print_error(cmd->argv[0], NULL, "command not found");
		exit(127);
	}
	envp = exported_envp(shell->envp);
	execve(path, cmd->argv, envp);
	if (envp != shell->envp)
		free(envp);
	perror(path);
	exit(126);
}
