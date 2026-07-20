/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execute.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 15:41:28 by Visual            #+#    #+#             */
/*   Updated: 2026/07/20 15:57:43 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	execute(t_shell *shell)
{
	if (count_cmds(shell->cmds) == 1)
		return (run_single(shell, shell->cmds));
	return (run_pipeline(shell));
}
