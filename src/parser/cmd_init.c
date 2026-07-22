/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 09:41:52 by Daniela           #+#    #+#             */
/*   Updated: 2026/07/22 15:50:42 by Visual           ###   ########.fr       */
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

int	count_args(t_token *tok)
{
	int	count;

	count = 0;
	while (tok && tok->type == TOK_WORD)
	{
		count++;
		while (tok->join && tok->next)
			tok = tok->next;
		tok = tok->next;
	}
	return (count);
}

static char	*merge_word(t_token **tok)
{
	t_token	*cur;
	char	*res;
	char	*tmp;

	cur = *tok;
	res = ft_strdup(cur->value);
	while (res && cur->join && cur->next)
	{
		cur = cur->next;
		tmp = ft_strjoin(res, cur->value);
		free(res);
		res = tmp;
	}
	*tok = cur->next;
	return (res);
}

char	**make_argv(t_token *tok, int count)
{
	char	**argv;
	int		i;

	argv = malloc(sizeof(char *) * (count + 1));
	if (argv == NULL)
		return (NULL);
	i = 0;
	while (i < count)
	{
		argv[i] = merge_word(&tok);
		if (argv[i] == NULL)
		{
			free_str_array(argv);
			return (NULL);
		}
		i++;
	}
	argv[i] = NULL;
	return (argv);
}
