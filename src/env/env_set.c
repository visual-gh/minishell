/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_set.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:55:15 by Visual            #+#    #+#             */
/*   Updated: 2026/05/08 16:01:53 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*make_entry(const char *key, const char *val)
{
	char	*entry;
	size_t	klen;
	size_t	vlen;

	klen = ft_strlen(key);
	vlen = ft_strlen(val);
	entry = malloc(klen + vlen + 2);
	if (!entry)
		return (NULL);
	ft_memcpy(entry, key, klen);
	entry[klen] = '=';
	ft_memcpy(entry + klen + 1, val, vlen + 1);
	return (entry);
}

static int	env_find(char **envp, const char *key, int *n)
{
	size_t	klen;

	klen = ft_strlen(key);
	*n = 0;
	while (envp[*n])
	{
		if (ft_strncmp(envp[*n], key, klen) == 0 && envp[*n][klen] == '=')
			return (1);
		(*n)++;
	}
	return (0);
}

static int	env_append(char ***envp, char *entry, int n)
{
	char	**new;

	new = ft_calloc(n + 2, sizeof(char *));
	if (!new)
	{
		free(entry);
		return (-1);
	}
	ft_memcpy(new, *envp, n * sizeof(char *));
	new[n] = entry;
	free(*envp);
	*envp = new;
	return (0);
}

int	env_set(char ***envp, const char *key, const char *val)
{
	char	*entry;
	int		n;

	entry = make_entry(key, val);
	if (!entry)
		return (-1);
	if (env_find(*envp, key, &n))
	{
		free((*envp)[n]);
		(*envp)[n] = entry;
		return (0);
	}
	return (env_append(envp, entry, n));
}
