/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_scan.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 11:45:23 by Daniela           #+#    #+#             */
/*   Updated: 2026/07/22 15:40:16 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	add_word_token(t_token **head, char *value, int quoted_type)
{
	t_token	*tok;

	tok = token_init(TOK_WORD, value, quoted_type);
	free(value);
	if (tok == NULL)
		return (-1);
	add_token(head, tok);
	return (0);
}

static int	scan_quote(char *input, int i, char quote_char)
{
	while (input[i] && input[i] != quote_char)
		i++;
	return (i);
}

int	if_quotes(t_token **head, int *i, char *input)
{
	char	quote_char;
	int		quoted_type;
	int		start;
	int		end;
	char	*value;

	quote_char = input[*i];
	if (quote_char == '\'')
		quoted_type = 1;
	else
		quoted_type = 2;
	(*i)++;
	start = *i;
	end = scan_quote(input, start, quote_char);
	if (input[end] != quote_char)
	{
		print_error(NULL, NULL, "unclosed quote");
		return (-1);
	}
	value = ft_substr(input, start, end - start);
	if (value == NULL || add_word_token(head, value, quoted_type) == -1)
		return (-1);
	*i = end + 1;
	return (0);
}

static int	scan_word(char *input, int i)
{
	while (input[i] && input[i] != ' ' && input[i] != '\t'
		&& input[i] != '\'' && input[i] != '"'
		&& is_separator(input, i) == TOK_WORD)
		i++;
	return (i);
}

int	if_word(t_token **head, int *i, char *input)
{
	int		start;
	int		end;
	char	*value;

	start = *i;
	end = scan_word(input, start);
	value = ft_substr(input, start, end - start);
	if (value == NULL || add_word_token(head, value, 0) == -1)
		return (-1);
	*i = end;
	return (0);
}
