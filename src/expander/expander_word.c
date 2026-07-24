/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_word.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 14:21:01 by Visual            #+#    #+#             */
/*   Updated: 2026/07/24 00:23:47 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*strjoin_free(char *s1, char *s2)
{
	char	*res;

	if (!s1 || !s2)
	{
		free(s1);
		return (NULL);
	}
	res = ft_strjoin(s1, s2);
	free(s1);
	return (res);
}

static char	*expand_single_quoted(char *word, int *i)
{
	int		start;
	char	*res;

	(*i)++;
	start = *i;
	while (word[*i] && word[*i] != '\'')
		(*i)++;
	res = ft_substr(word, start, *i - start);
	if (word[*i] == '\'')
		(*i)++;
	return (res);
}

static char	*scan_literal(char *word, int *i, char stop1, char stop2)
{
	int	start;

	start = *i;
	while (word[*i] && word[*i] != '$' && word[*i] != stop1
		&& word[*i] != stop2)
		(*i)++;
	return (ft_substr(word, start, *i - start));
}

static char	*expand_double_quoted(char *word, int *i, t_shell *shell)
{
	char	*res;
	char	*tmp;

	(*i)++;
	res = ft_strdup("");
	while (word[*i] && word[*i] != '"')
	{
		if (word[*i] == '$')
			tmp = expand_var(word, i, shell);
		else
			tmp = scan_literal(word, i, '"', '"');
		res = strjoin_free(res, tmp);
		free(tmp);
	}
	if (word[*i] == '"')
		(*i)++;
	return (res);
}

char	*expand_word(char *word, t_shell *shell)
{
	char	*res;
	char	*tmp;
	int		i;

	res = ft_strdup("");
	i = 0;
	while (word[i])
	{
		if (word[i] == '\'')
			tmp = expand_single_quoted(word, &i);
		else if (word[i] == '"')
			tmp = expand_double_quoted(word, &i, shell);
		else if (word[i] == '$')
			tmp = expand_var(word, &i, shell);
		else
			tmp = scan_literal(word, &i, '\'', '"');
		res = strjoin_free(res, tmp);
		free(tmp);
	}
	return (res);
}
