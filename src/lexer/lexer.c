#include "minishell.h"


int	is_separator(char *str, int i)
{
	if (str[i] == '|')
		return (TOK_PIPE);
	else if (str[i] == '<' && str[i + 1] == '<')
		return (TOK_HEREDOC);
	else if (str[i] == '>' && str[i + 1] == '>')
		return (TOK_APPEND);
	else if (str[i] == '<')
		return (TOK_REDIR_IN);
	else if (str[i] == '>')
		return (TOK_REDIR_OUT);
	else 
		return (TOK_WORD);
}

t_token	*lexer(char *input)
{
	t_token *head;
	int		tok;
	int		i;

	head = NULL;
	i = 0;
	while (input[i] != '\0')
	{
		while (input[i] == ' ' || input[i] == '/t')
			i++;
		if (input[i] == '\0')
			break;
		tok = is_separator(input, i);
	}
}