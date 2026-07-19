/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export_print.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/17 19:18:00 by Visual            #+#    #+#             */
/*   Updated: 2026/07/19 14:54:19 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	swap_str(char **a, char **b)
{
	char	*tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static void	sort_str_array(char **arr, int count)
{
	int	i;
	int	j;

	i = 0;
	while (i < count - 1)
	{
		j = 0;
		while (j < count - 1 - i)
		{
			if (ft_strncmp(arr[j], arr[j + 1], SIZE_MAX) > 0)
				swap_str(&arr[j], &arr[j + 1]);
			j++;
		}
		i++;
	}
}

static char	**sorted_envp_copy(char **envp)
{
	char	**copy;
	int		count;
	int		i;

	count = 0;
	while (envp[count])
		count++;
	copy = malloc(sizeof(char *) * (count + 1));
	if (!copy)
		return (NULL);
	i = 0;
	while (i <= count)
	{
		copy[i] = envp[i];
		i++;
	}
	sort_str_array(copy, count);
	return (copy);
}

static void	print_entry(char *entry)
{
	char	*eq;

	eq = ft_strchr(entry, '=');
	ft_putstr_fd("declare -x ", STDOUT_FILENO);
	if (!eq)
	{
		ft_putendl_fd(entry, STDOUT_FILENO);
		return ;
	}
	*eq = '\0';
	ft_putstr_fd(entry, STDOUT_FILENO);
	*eq = '=';
	ft_putstr_fd("=\"", STDOUT_FILENO);
	ft_putstr_fd(eq + 1, STDOUT_FILENO);
	ft_putendl_fd("\"", STDOUT_FILENO);
}

void	print_export(char **envp)
{
	char	**sorted;
	int		i;

	sorted = sorted_envp_copy(envp);
	if (!sorted)
		return ;
	i = 0;
	while (sorted[i])
		print_entry(sorted[i++]);
	free(sorted);
}
