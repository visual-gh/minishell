/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resolve_path.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 16:03:46 by Visual            #+#    #+#             */
/*   Updated: 2026/07/19 17:49:23 by Visual           ###   ########.fr       */
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

char	*resolve_path(char *cmd, char **envp)
{
	char	*path;
	char	**dirs;
	char	*full;
	int		i;

	if (ft_strchr(cmd, '/'))
		return (resolve_slash(cmd));
	path = env_get(envp, "PATH");
	if (!path)
		return (NULL);
	dirs = ft_split(path, ':');
	if (!dirs)
		return (NULL);
	i = 0;
	while (dirs[i])
	{
		full = path_join(dirs[i], cmd);
		if (full && access(full, X_OK) == 0)
			return (free_str_array(dirs), full);
		free(full);
		i++;
	}
	return (free_str_array(dirs), NULL);
}
