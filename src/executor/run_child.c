/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_child.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 18:52:16 by Visual            #+#    #+#             */
/*   Updated: 2026/08/04 19:46:32 by Visual           ###   ########.fr       */
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

static void	child_exit(t_shell *shell, int status)
{
	shell_free(shell);
	exit(status);
}

static void	exec_fail(t_shell *shell, char *path, int err)
{
	struct stat	st;

	errno = err;
	if (err == EACCES && stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		errno = EISDIR;
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	perror(path);
	free(path);
	child_exit(shell, 126);
}

void	run_child(t_shell *shell, t_cmd *cmd)
{
	char	*path;
	char	**envp;
	int		err;

	signals_child();
	shell->in_child = 1;
	if (apply_redirs(cmd->redirs) < 0)
		child_exit(shell, 1);
	if (!cmd->argv || !cmd->argv[0])
		child_exit(shell, 0);
	if (is_builtin(cmd->argv[0]))
		child_exit(shell, run_builtin(cmd, shell));
	path = resolve_path(cmd->argv[0], shell->envp);
	if (!path)
	{
		print_error(cmd->argv[0], NULL,
			not_found_msg(cmd->argv[0], shell->envp));
		child_exit(shell, 127);
	}
	envp = exported_envp(shell->envp);
	execve(path, cmd->argv, envp);
	err = errno;
	if (envp != shell->envp)
		free(envp);
	exec_fail(shell, path, err);
}
