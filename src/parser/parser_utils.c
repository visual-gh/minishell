#include "minishell.h"

//create commands
//walk through the tokens
//if there is a redirection i need to sent it to the next target 
// if its a pipe do the first command and create a new one

static t_redir_new(t_redir_type type, char *target, int quoted)
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