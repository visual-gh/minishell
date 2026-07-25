/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 20:20:38 by Daniela           #+#    #+#             */
/*   Updated: 2026/07/25 02:04:07 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	separator_type(char *str, int i)
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

static int	handle_token(t_token **head, t_token **tail, int *i, char *input)
{
	int	tok;

	tok = separator_type(input, *i);
	if (tok == TOK_PIPE)
		return (lex_pipe(head, tail, i));
	else if (tok == TOK_HEREDOC || tok == TOK_APPEND
		|| tok == TOK_REDIR_IN || tok == TOK_REDIR_OUT)
		return (lex_redir(head, tail, i, tok));
	else
		return (lex_word(head, tail, i, input));
}

t_token	*lexer(char *input)
{
	t_token	*head;
	t_token	*tail;
	int		i;

	head = NULL;
	tail = NULL;
	i = 0;
	while (input[i] != '\0')
	{
		while (input[i] == ' ' || input[i] == '\t')
			i++;
		if (input[i] == '\0')
			break ;
		if (handle_token(&head, &tail, &i, input) == -1)
		{
			free_tokens(head);
			return (NULL);
		}
	}
	return (head);
}
