/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_unset.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 18:01:23 by Visual            #+#    #+#             */
/*   Updated: 2026/07/24 20:05:16 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	env_unset(char ***envp, const char *key)
{
	int	i;

	i = env_index(*envp, key);
	if (i < 0)
		return ;
	free((*envp)[i]);
	while ((*envp)[i + 1])
	{
		(*envp)[i] = (*envp)[i + 1];
		i++;
	}
	(*envp)[i] = NULL;
}
