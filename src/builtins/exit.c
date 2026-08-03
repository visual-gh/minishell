/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/17 19:52:55 by Visual            #+#    #+#             */
/*   Updated: 2026/08/03 16:37:18 by Visual           ###   ########.fr       */
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

static int	safe_atol(char *str, long *out)
{
	unsigned long	res;
	unsigned long	limit;
	int				neg;

	neg = 0;
	if (*str == '-' || *str == '+')
		neg = (*str++ == '-');
	limit = (unsigned long)LONG_MAX + neg;
	res = 0;
	while (*str)
	{
		if (res > (limit - (*str - '0')) / 10)
			return (0);
		res = res * 10 + (*str++ - '0');
	}
	if (neg && res > 0)
		*out = -(long)(res - 1) - 1;
	else
		*out = (long)res;
	return (1);
}

static void	print_exit(t_shell *shell)
{
	if (isatty(STDIN_FILENO) && !shell->in_child)
		ft_putstr_fd("exit\n", STDERR_FILENO);
}

int	ft_exit(t_cmd *cmd, t_shell *shell)
{
	long	status;

	print_exit(shell);
	status = shell->last_status;
	if (cmd->argv[1] && (!is_numeric(cmd->argv[1])
			|| !safe_atol(cmd->argv[1], &status)))
	{
		print_error("exit", cmd->argv[1], "numeric argument required");
		shell_free(shell);
		exit(2);
	}
	if (cmd->argv[1] && cmd->argv[2])
		return (print_error("exit", NULL, "too many arguments"), 1);
	shell_free(shell);
	exit(status & 255);
}
