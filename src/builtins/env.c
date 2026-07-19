/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/17 19:46:08 by Visual            #+#    #+#             */
/*   Updated: 2026/07/17 19:46:45 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	ft_env(t_shell *shell)
{
	int	i;

	i = 0;
	while (shell->envp[i])
		ft_putendl_fd(shell->envp[i++], STDOUT_FILENO);
	return (0);
}
