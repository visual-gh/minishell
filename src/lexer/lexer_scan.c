/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_scan.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 11:45:23 by Daniela           #+#    #+#             */
/*   Updated: 2026/07/25 02:04:47 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	add_word_token(t_token **head, t_token **tail, char *value)
{
	t_token	*tok;

	tok = token_init(TOK_WORD, value);
	free(value);
	if (tok == NULL)
		return (-1);
	add_token(head, tail, tok);
	return (0);
}

static int	is_word_char(char *input, int i, char quote)
{
	if (quote)
		return (1);
	if (input[i] == ' ' || input[i] == '\t')
		return (0);
	return (separator_type(input, i) == TOK_WORD);
}

static int	scan_word(char *input, int i, int *err)
{
	char	quote;

	quote = 0;
	while (input[i] && is_word_char(input, i, quote))
	{
		if (quote)
		{
			if (input[i] == quote)
				quote = 0;
		}
		else if (input[i] == '\'' || input[i] == '"')
			quote = input[i];
		i++;
	}
	if (quote)
		*err = 1;
	return (i);
}

int	lex_word(t_token **head, t_token **tail, int *i, char *input)
{
	int		start;
	int		end;
	int		err;
	char	*value;

	start = *i;
	err = 0;
	end = scan_word(input, start, &err);
	if (err)
	{
		print_error(NULL, NULL, "unclosed quote");
		return (-1);
	}
	value = ft_substr(input, start, end - start);
	if (value == NULL || add_word_token(head, tail, value) == -1)
		return (-1);
	*i = end;
	return (0);
}
