#include "minishell.h"


int    is_separator(char *str, int i)
{
    if (str[i] == '|')
        return (TOK_PIPE);
    else if (str[i] == '<' && str[i + 1] && str[i + 1] == '<')
        return (TOK_HEREDOC);
    else if (str[i] == '>' && str[i + 1] && str[i + 1] == '>')
        return (TOK_APPEND);
    else if (str[i] == '<')
        return (TOK_REDIR_IN);
    else if (str[i] == '>')
        return (TOK_REDIR_OUT);
    else
        return (TOK_WORD);
}
 



static int    handle_token(t_token **head, int *i, char *input)
{
    int tok;

    tok = is_separator(input, *i);
    if (tok == TOK_PIPE)
        return (if_pipe(head, i));
    else if (tok == TOK_HEREDOC || tok == TOK_APPEND
        || tok == TOK_REDIR_IN || tok == TOK_REDIR_OUT)
        return (if_redir(head, i, tok));
    else if (input[*i] == '\'' || input[*i] == '"')
        return (if_quotes(head, i, input));
    else
        return (if_word(head, i, input));
}



static void    set_join(t_token *head, char *input, int i)
{
    t_token *last;

    last = head;
    while (last->next != NULL) 
        last = last->next;
    if (input[i] != '\0' && input[i] != ' ' && input[i] != '\t'
        && input[i] != '|' && input[i] != '<' && input[i] != '>')
        last->join = 1;
    else
        last->join = 0;
}



static int    lexer_step(t_token **head, char *input, int *i)
{
    if (handle_token(head, i, input) == -1)
        return (-1);
    if (*head != NULL)
        set_join(*head, input, *i);
    return (0);                             
}


t_token    *lexer(char *input)
{
    t_token *head;
    int     i;

    head = NULL;
    i = 0;
    while (input[i] != '\0')
    {
        while (input[i] == ' ' || input[i] == '\t')
            i++;
        if (input[i] == '\0')
            break ;
        if (lexer_step(&head, input, &i) == -1)
        {
            free_tokens(head);
            return (NULL);
        }
    }
    return (head);
}