/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 19:57:20 by Visual            #+#    #+#             */
/*   Updated: 2025/10/04 00:59:54 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_word_len(char const *s, char c)
{
	int	len;

	len = 0;
	while (s[len] && (s[len] != c))
		len++;
	return (len);
}

static int	ft_count_words(char const *s, char c)
{
	int	count;

	count = 0;
	while (*s)
	{
		while (*s && (*s == c))
			s++;
		if (*s)
		{
			count++;
			while (*s && (*s != c))
				s++;
		}
	}
	return (count);
}

static char	**ft_fill(char **split, char const *s, char c, int count)
{
	int	i;
	int	len;

	i = 0;
	while (i < count)
	{
		while (*s && (*s == c))
			s++;
		len = ft_word_len(s, c);
		split[i] = ft_substr(s, 0, len);
		if (!split[i])
		{
			while (i--)
				free(split[i]);
			free(split);
			return (NULL);
		}
		s += len;
		i++;
	}
	split[i] = 0;
	return (split);
}

char	**ft_split(char const *s, char c)
{
	char	**split;
	int		count;

	if (!s)
		return (NULL);
	count = ft_count_words(s, c);
	split = malloc(sizeof(char *) * (count + 1));
	if (!split)
		return (NULL);
	return (ft_fill(split, s, c, count));
}
