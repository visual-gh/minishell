/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 20:20:38 by Daniela           #+#    #+#             */
/*   Updated: 2026/07/24 16:30:12 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_separator(char *str, int i)
{
	if (str[i] == '|')
		return (TOK_PIPE);
	else if (str[i] == '<' && str[i + 1] && str[i + 1] == '<')
		return (TOK_HEREDOC);
	else if (str[i] == '>' && str[i + 1] && str[i + 1] == '>')
		return (TOK_APPEND);
	else if (str[i] == '<')
		return (TOK_REDIR_IN);
	else if (str[i] == '>')
		return (TOK_REDIR_OUT);
	else
		return (TOK_WORD);
}

static int	handle_token(t_token **head, int *i, char *input)
{
	int	tok;

	tok = is_separator(input, *i);
	if (tok == TOK_PIPE)
		return (if_pipe(head, i));
	else if (tok == TOK_HEREDOC || tok == TOK_APPEND
		|| tok == TOK_REDIR_IN || tok == TOK_REDIR_OUT)
		return (if_redir(head, i, tok));
	else
		return (if_word(head, i, input));
}

t_token	*lexer(char *input)
{
	t_token	*head;
	int		i;

	head = NULL;
	i = 0;
	while (input[i] != '\0')
	{
		while (input[i] == ' ' || input[i] == '\t')
			i++;
		if (input[i] == '\0')
			break ;
		if (handle_token(&head, &i, input) == -1)
		{
			free_tokens(head);
			return (NULL);
		}
	}
	return (head);
}
