#include "minishell.h"


//file errors

void    free_tokens(t_token *head)
{
    t_token *tmp;

    while (head != NULL)
    {
        tmp = head->next;
        free(head->value); 
        free(head);
        head = tmp; 
    }
}

int    print_error(char *cmd, char *arg, char *msg)
{
    ft_putstr_fd("minishell: ", 2);
    if (cmd)
    {
        ft_putstr_fd(cmd, 2);
        ft_putstr_fd(": ", 2);
    }
    if (arg)
    {
        ft_putstr_fd(arg, 2); 
        ft_putstr_fd(": ", 2);
    }
    ft_putstr_fd(msg, 2);
    ft_putstr_fd("\n", 2);
    return (-1); 
}