/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_init.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 16:22:18 by Daniela           #+#    #+#             */
/*   Updated: 2026/07/22 16:14:58 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_token	*token_init(t_tok_type type, char *value, int quoted)
{
	t_token	*token;

	token = malloc(sizeof(t_token));
	if (token == NULL)
		return (NULL);
	token->type = type;
	if (value != NULL)
	{
		token->value = ft_strdup(value);
		if (token->value == NULL)
		{
			free(token);
			return (NULL);
		}
	}
	else
		token->value = NULL;
	token->quoted = quoted;
	token->join = 0;
	token->next = NULL;
	return (token);
}

void	add_token(t_token **head, t_token *new_node)
{
	t_token	*current;

	if (*head == NULL)
	{
		*head = new_node;
		return ;
	}
	current = *head;
	while (current->next != NULL)
		current = current->next;
	current->next = new_node;
}
