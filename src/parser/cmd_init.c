/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 09:41:52 by Daniela           #+#    #+#             */
/*   Updated: 2026/07/24 05:52:43 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_cmd	*cmd_new(void)
{
	t_cmd	*cmd;

	cmd = malloc(sizeof(t_cmd));
	if (cmd == NULL)
		return (NULL);
	cmd->argv = NULL;
	cmd->redirs = NULL;
	cmd->next = NULL;
	return (cmd);
}

void	add_cmd(t_cmd **head, t_cmd *new_cmd)
{
	t_cmd	*current;

	if (*head == NULL)
	{
		*head = new_cmd;
		return ;
	}
	current = *head;
	while (current->next != NULL)
		current = current->next;
	current->next = new_cmd;
}

int	count_all_words(t_token *tok)
{
	int	count;

	count = 0;
	while (tok && tok->type != TOK_PIPE)
	{
		if (tok->type == TOK_WORD)
		{
			count++;
			tok = tok->next;
		}
		else
		{
			tok = tok->next;
			if (tok)
				tok = tok->next;
		}
	}
	return (count);
}
