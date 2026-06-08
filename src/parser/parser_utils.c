#include "minishell.h"

//create commands
//walk through the tokens
//if there is a redirection i need to sent it to the next target 
// if its a pipe do the first command and create a new one

static t_redir  *redir_new(t_redir_type type, char *target, int quoted)
{
    t_redir     *redir;

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
    redir->next = NULL;
    return (redir);
}

int add_redir(t_cmd *cmd, t_redir_type type, char *target, int quoted)
{
    t_redir *redir;
    t_redir *current;

    redir = redir_new(type, target, quoted);
    if (redir == NULL)
        return (-1);
    if (cmd->redirs == NULL)
    {
        
    }
}


static int	tok_to_red(t_tok_type type)
{
	if (type == TOK_REDIR_IN)
		return (REDIR_IN);
	else if (type == TOK_REDIR_OUT)
		return (REDIR_OUT);
	else if (type == TOK_APPEND)
		return (REDIR_APPEND);
	else
		return (REDIR_HEREDOC);
}


int	parse_redir(t_cmd *cmd, t_token **tok)
{
	t_redir_type	type;
	t_token			*target;
	
	type = tok_to_red((*tok)->type);
	target = (*tok)->next;
	if (target == NULL || target->type != TOK_WORD)
		return (1);// need a print error
	if (add_redir(cmd, type, target->value, target->quoted))
		return (-1);
	*tok = target->next;
	return (0);
}

int 	parse_word(t_cmd *cmd, t_token **tok)
{
	t_token	*tmp;
	int		count;
	int		i;

	tmp = *tok;
	count = 0;
	while (tmp && tmp->type == TOK_WORD)
	{
		count++;
		if (!tmp->join)
			break ;
		tmp = tmp->next;
	}
	cmd->argv = make_argv(*tok, count);
	if (cmd->argv == NULL)
		return (-1);
	i = 0;
	while (i < count)
	{
		*tok = (*tok)->next;
		i++;
	}
	return (0);
}

static int	parse_token(t_cmd *cmd, t_token **tok)
{
	if ((*tok)->type == TOK_REDIR_IN || (*tok)->type == TOK_REDIR_OUT || (*tok)->type == TOK_APPEND || (*tok)->type == TOK_HEREDOC)
		return (parse_redir(cmd, tok));
	else if ((*tok)->type == TOK_WORD)
		return (parse_word(cmd, tok));
}

//redir new ---
//add redir---
//parse redir ----
//parse word
//parse token