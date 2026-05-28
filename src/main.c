/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 13:24:17 by Visual            #+#    #+#             */
/*   Updated: 2026/05/06 19:19:37 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	main(int argc, char **argv, char **envp)
{
	t_shell	*shell;
	int		status;

	(void)argv;
	if (argc != 1)
	{
		print_error(NULL, NULL, "no arguments expected");
		return (1);
	}
	shell = shell_init(envp);
	if (!shell)
		return (1);
	shell_loop(shell);
	status = shell->last_status;
	shell_free(shell);
	return (status);
}
