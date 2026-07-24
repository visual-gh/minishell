/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 11:45:23 by Daniela           #+#    #+#             */
/*   Updated: 2026/07/24 16:34:08 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	if_pipe(t_token **head, int *i)
{
	t_token	*tok;

	tok = token_init(TOK_PIPE, NULL);
	if (tok == NULL)
		return (-1);
	add_token(head, tok);
	(*i) += 1;
	return (0);
}

int	if_redir(t_token **head, int *i, int sep)
{
	t_token	*tok;

	tok = token_init(sep, NULL);
	if (tok == NULL)
		return (-1);
	add_token(head, tok);
	if (sep == TOK_HEREDOC || sep == TOK_APPEND)
		(*i) += 2;
	else
		(*i) += 1;
	return (0);
}
