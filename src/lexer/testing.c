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
	t_token	*head;
	t_token	*tok;

	head = NULL;

	tok = token_init(TOK_WORD, "echo", 0);
	add_token(&head, tok);

	tok = token_init(TOK_PIPE, NULL, 0);
	add_token(&head, tok);

	tok = token_init(TOK_WORD, "hello world", 1);
	add_token(&head, tok);

	tok = token_init(TOK_WORD, "$USER", 2);
	add_token(&head, tok);

	print_tokens(head);
	return (0);
}