/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 09:41:52 by Daniela           #+#    #+#             */
/*   Updated: 2026/07/25 01:08:56 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static t_redir	*redir_new(t_redir_type type, char *target, int quoted)
{
	t_redir	*redir;

	redir = malloc(sizeof(t_redir));
	if (redir == NULL)
		return (NULL);
	redir->type = type;
	redir->target = ft_strdup(target);
	if (redir->target == NULL)
	{
		free(redir);
		return (NULL);
	}
	redir->quoted = quoted;
	redir->heredoc_fd = -1;
	redir->next = NULL;
	return (redir);
}

int	add_redir(t_cmd *cmd, t_redir_type type, char *target, int quoted)
{
	t_redir	*redir;
	t_redir	*current;

	redir = redir_new(type, target, quoted);
	if (redir == NULL)
		return (-1);
	if (cmd->redirs == NULL)
		cmd->redirs = redir;
	else
	{
		current = cmd->redirs;
		while (current->next != NULL)
			current = current->next;
		current->next = redir;
	}
	return (0);
}

static int	has_quote(char *raw)
{
	int	i;

	i = 0;
	while (raw[i])
	{
		if (raw[i] == '\'' || raw[i] == '"')
			return (1);
		i++;
	}
	return (0);
}

static void	strip_char(char c, char *res, int *j, char *quote)
{
	if (*quote)
	{
		if (c == *quote)
			*quote = 0;
		else
			res[(*j)++] = c;
	}
	else if (c == '\'' || c == '"')
		*quote = c;
	else
		res[(*j)++] = c;
}

char	*strip_quotes(char *raw, int *quoted)
{
	char	*res;
	int		i;
	int		j;
	char	quote;

	res = malloc(sizeof(char) * (ft_strlen(raw) + 1));
	if (res == NULL)
		return (NULL);
	*quoted = has_quote(raw);
	i = 0;
	j = 0;
	quote = 0;
	while (raw[i])
	{
		strip_char(raw[i], res, &j, &quote);
		i++;
	}
	res[j] = '\0';
	return (res);
}
