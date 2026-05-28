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