#include "minishell.h"

//create tokens and adds them to the list


void	if_pipe(t_token **head, int *i) //used pointer to the list so can modify it  and also a pointer to the int/position in the input str
{
	t_token	*tok;

	tok = token_init(TOK_PIPE, NULL, 0); // its not quoted so stays at 0
	if (tok == NULL) // dont think i need it since i check for malloc failiur in the init function but stays here for now
		return;
	add_token(head, tok);
	(*i) += 1;
}

