/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_expand.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 04:09:20 by Visual            #+#    #+#             */
/*   Updated: 2026/07/24 00:59:31 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*scan_heredoc_literal(char *line, int *i)
{
	int	start;

	start = *i;
	while (line[*i] && line[*i] != '$')
		(*i)++;
	return (ft_substr(line, start, *i - start));
}

char	*expand_heredoc_line(char *line, t_shell *shell)
{
	char	*res;
	char	*tmp;
	char	*next;
	int		i;

	res = ft_strdup("");
	i = 0;
	while (line[i])
	{
		if (line[i] == '$')
			tmp = expand_var(line, &i, shell);
		else
			tmp = scan_heredoc_literal(line, &i);
		next = ft_strjoin(res, tmp);
		free(res);
		free(tmp);
		res = next;
	}
	return (res);
}
