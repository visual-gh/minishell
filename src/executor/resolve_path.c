/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resolve_path.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 16:03:46 by Visual            #+#    #+#             */
/*   Updated: 2026/08/03 19:56:37 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*path_join(char *dir, char *cmd)
{
	char	*tmp;
	char	*full;

	tmp = ft_strjoin(dir, "/");
	if (!tmp)
		return (NULL);
	full = ft_strjoin(tmp, cmd);
	free(tmp);
	return (full);
}

static char	*resolve_slash(char *cmd)
{
	if (access(cmd, F_OK) == 0)
		return (ft_strdup(cmd));
	return (NULL);
}

static char	*search_dirs(char **dirs, char *cmd)
{
	char	*full;
	char	*denied;
	int		i;

	denied = NULL;
	i = 0;
	while (dirs[i])
	{
		full = path_join(dirs[i], cmd);
		if (full && access(full, X_OK) == 0)
			return (free(denied), full);
		if (full && denied == NULL && access(full, F_OK) == 0)
			denied = full;
		else
			free(full);
		i++;
	}
	return (denied);
}

char	*not_found_msg(char *cmd, char **envp)
{
	char	*path;

	path = env_get(envp, "PATH");
	if (ft_strchr(cmd, '/') || !path || !*path)
		return (strerror(ENOENT));
	return ("command not found");
}

char	*resolve_path(char *cmd, char **envp)
{
	char	*path;
	char	**dirs;
	char	*full;

	if (!*cmd)
		return (NULL);
	if (ft_strchr(cmd, '/'))
		return (resolve_slash(cmd));
	path = env_get(envp, "PATH");
	if (!path)
		return (NULL);
	dirs = ft_split(path, ':');
	if (!dirs)
		return (NULL);
	full = search_dirs(dirs, cmd);
	return (free_str_array(dirs), full);
}
