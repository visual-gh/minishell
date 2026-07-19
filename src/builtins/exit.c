/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/17 19:52:55 by Visual            #+#    #+#             */
/*   Updated: 2026/07/19 15:13:18 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_numeric(char *str)
{
	int	i;

	i = 0;
	if (str[i] == '+' || str[i] == '-')
		i++;
	if (!str[i])
		return (0);
	while (str[i])
		if (!ft_isdigit(str[i++]))
			return (0);
	return (1);
}

int	ft_exit(t_cmd *cmd, t_shell *shell)
{
	int	status;

	if (!cmd->argv[1])
		status = shell->last_status;
	else if (!is_numeric(cmd->argv[1]))
	{
		print_error("exit", cmd->argv[1], "numeric argument required");
		shell_free(shell);
		exit(2);
	}
	else if (cmd->argv[2])
		return (print_error("exit", NULL, "too many arguments"), 1);
	else
		status = ft_atoi(cmd->argv[1]);
	shell_free(shell);
	exit(status);
}
