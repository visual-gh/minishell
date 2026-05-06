/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shell_init.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 18:38:56 by Visual            #+#    #+#             */
/*   Updated: 2026/05/06 20:06:48 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_shell	*shell_init(char **envp)
{
	t_shell	*shell;

	(void)envp;
	shell = ft_calloc(1, sizeof(t_shell));
	if (!shell)
		return (NULL);
	signals_prompt();
	return (shell);
}
