/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_child.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 18:52:16 by Visual            #+#    #+#             */
/*   Updated: 2026/07/25 02:09:11 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	run_child(t_shell *shell, t_cmd *cmd)
{
	char	*path;

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
	execve(path, cmd->argv, shell->envp);
	perror(path);
	exit(126);
}
