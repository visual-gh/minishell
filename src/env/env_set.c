/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_set.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:55:15 by Visual            #+#    #+#             */
/*   Updated: 2026/07/27 17:59:11 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*make_entry(const char *key, const char *val)
{
	char	*entry;
	size_t	klen;
	size_t	vlen;

	if (!val)
		return (ft_strdup(key));
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
	n = env_index(*envp, key);
	if (n >= 0)
	{
		free((*envp)[n]);
		(*envp)[n] = entry;
		return (0);
	}
	n = 0;
	while ((*envp)[n])
		n++;
	return (env_append(envp, entry, n));
}
