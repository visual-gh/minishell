/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/17 19:12:46 by Visual            #+#    #+#             */
/*   Updated: 2026/07/19 15:07:58 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_valid_identificator(char *str)
{
	int	i;

	if (!ft_isalpha(str[0]) && str[0] != '_')
		return (0);
	i = 1;
	while (str[i] && str[i] != '=')
	{
		if (!ft_isalnum(str[i]) && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

static int	export_one(char *arg, t_shell *shell)
{
	char	*eq;
	char	*key;
	int		ret;

	if (!is_valid_identificator(arg))
		return (print_error("export", arg, "not a valid identifier"), 1);
	eq = ft_strchr(arg, '=');
	if (eq == NULL)
	{
		if (env_get(shell->envp, arg) == NULL)
			env_set(&shell->envp, arg, "");
		return (0);
	}
	key = ft_substr(arg, 0, eq - arg);
	if (key == NULL)
		return (1);
	ret = env_set(&shell->envp, key, eq + 1);
	free(key);
	return (ret != 0);
}

int	ft_export(t_cmd *cmd, t_shell *shell)
{
	int	i;
	int	status;

	if (cmd->argv[1] == NULL)
		return (print_export(shell->envp), 0);
	i = 1;
	status = 0;
	while (cmd->argv[i])
		if (export_one(cmd->argv[i++], shell))
			status = 1;
	return (status);
}
