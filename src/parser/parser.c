#include "minishell.h"

static int	parse_token(t_cmd *cmd, t_token **tok)
{
	if ((*tok)->type == TOK_REDIR_IN || (*tok)->type == TOK_REDIR_OUT || (*tok)->type == TOK_APPEND || (*tok)->type == TOK_HEREDOC)
		return (parse_redir(cmd, tok));
	else if ((*tok)->type == TOK_WORD)
		return (parse_word(cmd, tok));
	return (0);
}

static int	parse_pipeline(t_cmd **cmds, t_token **tok)
{
	t_cmd	*cmd;

	cmd = cmd_new();
	if (cmd == NULL)
		return (-1);
	while (*tok && (*tok)->type != TOK_PIPE)
	{
		if (parse_token(cmd, tok) == -1)
		{
			//free
			return (-1);
		}
	}
	add_cmd(cmds, cmd);
	if (*tok && (*tok)->type == TOK_PIPE)
		*tok = (*tok)->next;
	return (0);
}

int 	parse(t_token *tokens, t_shell *shell)
{
	t_token	*tok;

	tok = tokens;
	shell->cmds = NULL;
	while (tok != NULL)
	{
		if (parse_pipeline(&shell->cmds, &tok) == -1)
		{
			//free
			shell->cmds = NULL;
			return (-1);
		}
	}
	return (0);
}