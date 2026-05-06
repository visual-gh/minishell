#include "minishell.h"



//takes 3 args, the kind of token the content of the string and if its quoted, 
//ill return a pointer to a new token 

t_token *token_init(t_tok_type type, char *value, int quoted)
{
	t_token *token;

	token = malloc(sizeof(t_token));
	if (token == NULL)
		return (NULL);
	token->type = type; // store what kind of token it is
	if (value != NULL)  // if there is a stirnf make a copy of it so it doesent get lost if changed or freed
		token->value = ft_strdup(value);
	else
		token->value = NULL;
	token->quoted = quoted; // store if quoted or not, (0 = unquoted   1 = quoted and 2 = double quoted)
	token->join = 0;          // by default its 0 since its not joined to the next one, join will be set to one in the lexer if i dedect2 tokens next to each other with no space 
	token->next = NULL;		
	return (token);
}




void	add_token(t_token **head, t_token *new_node)
{
	t_token	*current;

	if (*head == NULL)
	{
		*head = new_node; // if the head is empty, the new token becomes the head
		return;
	}
	current = *head;
	while (current->next != NULL)
		current = current->next;
	current->next = new_node;

}
