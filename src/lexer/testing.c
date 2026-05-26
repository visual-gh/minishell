#include "minishell.h"

void	print_tokens(t_token *tokens)
{
	while (tokens != NULL)
	{
		printf("type: %d | value: %s | quoted: %d | join: %d\n",
			tokens->type, tokens->value, tokens->quoted, tokens->join);
		tokens = tokens->next;
	}
}


int	main(void)
{
	// t_token	*head;
	// t_token	*tok;

	// head = NULL;

	// tok = token_init(TOK_WORD, "echo", 0);
	// add_token(&head, tok);

	// tok = token_init(TOK_PIPE, NULL, 0);
	// add_token(&head, tok);

	// tok = token_init(TOK_WORD, "hello world", 1);
	// add_token(&head, tok);

	// tok = token_init(TOK_WORD, "$USER", 2);
	// add_token(&head, tok);

	// print_tokens(head);
	    char    *input;
    t_token *tokens;

    while (1)
    {
        input = readline("minishell> ");
        if (input == NULL)
            break ;
        tokens = lexer(input);
        if (tokens == NULL)
            ft_printf("lexer returned NULL\n");
        else
        {
            print_tokens(tokens);
            free_tokens(tokens);
        }
        free(input);
    }
	return (0);
}