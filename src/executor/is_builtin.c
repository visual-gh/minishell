/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   is_builtin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 15:38:42 by Visual            #+#    #+#             */
/*   Updated: 2026/07/19 15:47:32 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_builtin(char *name)
{
	if (!name)
		return (0);
	if (!ft_strncmp(name, "cd", SIZE_MAX)
		|| !ft_strncmp(name, "echo", SIZE_MAX)
		|| !ft_strncmp(name, "env", SIZE_MAX)
		|| !ft_strncmp(name, "exit", SIZE_MAX)
		|| !ft_strncmp(name, "export", SIZE_MAX)
		|| !ft_strncmp(name, "pwd", SIZE_MAX)
		|| !ft_strncmp(name, "unset", SIZE_MAX))
		return (1);
	return (0);
}
