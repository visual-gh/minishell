#include "minishell.h"

//create tokens and adds them to the list



int    if_pipe(t_token **head, int *i)
{
    t_token *tok;

    tok = token_init(TOK_PIPE, NULL, 0);
    if (tok == NULL)
        return (-1);
    add_token(head, tok);
    (*i) += 1;
    return (0);
}




int    if_redir(t_token **head, int *i, int sep)
{
    t_token *tok;

    tok = token_init(sep, NULL, 0);
    if (tok == NULL)
        return (-1);
    add_token(head, tok);
    if (sep == TOK_HEREDOC || sep == TOK_APPEND)
        (*i) += 2;
    else
        (*i) += 1;
    return (0);
}









static int    make_quote_token(t_token **head, char *input,
                int start, int end, int quoted_type)
{
    char    *value;
    t_token *tok;

    value = ft_substr(input, start, end - start);
    if (value == NULL)
        return (-1);
    tok = token_init(TOK_WORD, value, quoted_type);
    free(value);
    if (tok == NULL)
        return (-1);
    add_token(head, tok);
    return (0);
}






static int    scan_quote(char *input, int i, char quote_char)
{
    while (input[i] && input[i] != quote_char)
        i++;
    return (i);
}





int    if_quotes(t_token **head, int *i, char *input)
{
    char    quote_char;
    int     quoted_type;
    int     start;
    int     end;

    quote_char = input[*i];
    if (quote_char == '\'')
        quoted_type = 1;
    else
        quoted_type = 2;
    (*i)++;
    start = *i;
    end = scan_quote(input, start, quote_char);
    if (input[end] != quote_char)
        return (print_error(NULL, NULL, "unclosed quote"));
    if (make_quote_token(head, input, start, end, quoted_type) == -1)
        return (-1);
    *i = end + 1;
    return (0);
}

static int    scan_word(char *input, int i)
{
    while (input[i] && input[i] != ' ' && input[i] != '\t'
        && input[i] != '\'' && input[i] != '"'
        && is_separator(input, i) == TOK_WORD)
        i++;
    return (i);
}


int    if_word(t_token **head, int *i, char *input)
{
    int     start;
    int     end;
    char    *value;
    t_token *tok;

    start = *i;
    end = scan_word(input, start);
    value = ft_substr(input, start, end - start);
    if (value == NULL)
        return (-1);
    tok = token_init(TOK_WORD, value, 0);
    free(value);
    if (tok == NULL)
        return (-1);
    add_token(head, tok);
    *i = end;
    return (0);
}







