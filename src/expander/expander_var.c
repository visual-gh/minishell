/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_var.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 13:44:20 by Visual            #+#    #+#             */
/*   Updated: 2026/06/13 19:57:20 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	var_name_len(char *str, int i)
{
	int	len;

	len = 0;
	while (str[i + len] && (ft_isalnum(str[i + len]) || str[i + len] == '_'))
		len++;
	return (len);
}

static char	*expand_named_var(char *str, int *i, t_shell *shell)
{
	char	*name;
	char	*val;
	int		len;

	len = var_name_len(str, *i);
	name = ft_substr(str, *i, len);
	if (!name)
		return (NULL);
	*i += len;
	val = env_get(shell->envp, name);
	free(name);
	if (val)
		return (ft_strdup(val));
	return (ft_strdup(""));
}

char	*expand_var(char *str, int *i, t_shell *shell)
{
	(*i)++;
	if (!str[*i])
		return (ft_strdup("$"));
	if (str[*i] == '?')
	{
		(*i)++;
		return (ft_itoa(shell->last_status));
	}
	if (!ft_isalpha(str[*i]) && str[*i] != '_')
		return (ft_strdup("$"));
	return (expand_named_var(str, i, shell));
}
