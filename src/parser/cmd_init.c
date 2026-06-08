#include "minishell.h"



t_cmd    *cmd_new(void)
{
    t_cmd    *cmd;

    cmd = malloc(sizeof(t_cmd));
    if (cmd == NULL)
        return (NULL);
    cmd->argv = NULL;
    cmd->redirs = NULL;
    cmd->next = NULL;
    return (cmd);
}

void    add_cmd(t_cmd **head, t_cmd *new_cmd)  //same as add token 
{
    t_cmd    *current;

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


static int  count_args(t_token *tok)
{
    int count;

    count = 0;
    while (tok && tok->type == TOK_WORD) // walk for as long as there is word type tokens
    {
        count++;
        if (!tok->join)
            break;
        tok = tok->next;
    }
    return (count);
}

char    **make_argv(t_token *tok, int count)
{
    char    **argv;
    int     i;

    argv = malloc(sizeof(char *) * (count + 1));
    if (argv == NULL)
        return (NULL);
    i = 0;
    while (i < count)
    {
        argv[i] = ft_strdup(tok->value);
        if (argv[i] == NULL)
        {
            //free(); //need free str array
            return (NULL);
        }
        tok = tok->next;
        i++;
    } 
    argv[i] = NULL;
    return (argv);
}

//cmd new
//add cmd
//count args
//make argv
