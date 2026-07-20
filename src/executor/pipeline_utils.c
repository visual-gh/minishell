/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipeline_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 18:54:28 by Visual            #+#    #+#             */
/*   Updated: 2026/07/20 02:29:39 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	count_cmds(t_cmd *cmds)
{
	int	n;

	n = 0;
	while (cmds)
	{
		n++;
		cmds = cmds->next;
	}
	return (n);
}

int	**alloc_pipes(int n)
{
	int	**pipes;
	int	i;

	if (n <= 0)
		return (NULL);
	pipes = malloc(sizeof(int *) * (n + 1));
	if (!pipes)
		return (NULL);
	i = 0;
	while (i < n)
	{
		pipes[i] = malloc(sizeof(int) * 2);
		if (!pipes[i] || pipe(pipes[i]) < 0)
		{
			free(pipes[i]);
			pipes[i] = NULL;
			close_pipes(pipes);
			return (NULL);
		}
		i++;
	}
	pipes[n] = NULL;
	return (pipes);
}

void	close_pipes(int **pipes)
{
	int	i;

	if (!pipes)
		return ;
	i = 0;
	while (pipes[i])
	{
		close(pipes[i][0]);
		close(pipes[i][1]);
		free(pipes[i]);
		i++;
	}
	free(pipes);
}
