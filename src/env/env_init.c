/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 00:47:59 by Visual            #+#    #+#             */
/*   Updated: 2026/05/08 02:39:16 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	**env_init(char **envp)
{
	char	**copy;
	int		count;
	int		i;

	count = 0;
	while (envp[count])
		count++;
	copy = ft_calloc(count + 1, sizeof(char *));
	if (!copy)
		return (NULL);
	i = 0;
	while (i < count)
	{
		copy[i] = ft_strdup(envp[i]);
		if (!copy[i])
		{
			free_str_array(copy);
			return (NULL);
		}
		i++;
	}
	return (copy);
}
