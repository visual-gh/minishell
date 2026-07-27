/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_get.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 16:28:44 by Visual            #+#    #+#             */
/*   Updated: 2026/07/27 18:04:58 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	env_index(char **envp, const char *key)
{
	size_t	klen;
	int		i;

	klen = ft_strlen(key);
	i = 0;
	while (envp[i])
	{
		if (ft_strncmp(envp[i], key, klen) == 0
			&& (envp[i][klen] == '=' || envp[i][klen] == '\0'))
			return (i);
		i++;
	}
	return (-1);
}

char	*env_get(char **envp, const char *key)
{
	size_t	klen;
	int		i;

	i = env_index(envp, key);
	if (i < 0)
		return (NULL);
	klen = ft_strlen(key);
	if (envp[i][klen] != '=')
		return (NULL);
	return (envp[i] + klen + 1);
}
