/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shell_loop.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 18:56:11 by Visual            #+#    #+#             */
/*   Updated: 2026/07/22 16:41:33 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_blank(char *line)
{
	while (*line == ' ' || *line == '\t')
		line++;
	return (*line == '\0');
}

static void	process_line(char *line, t_shell *shell)
{
	t_token	*tokens;

	if (is_blank(line))
		return ;
	tokens = lexer(line);
	if (!tokens)
	{
		shell->last_status = 2;
		return ;
	}
	if (parse(tokens, shell) == -1)
		shell->last_status = 2;
	else if (shell->cmds && read_heredocs(shell) && expand_cmds(shell))
		shell->last_status = execute(shell);
	free_tokens(tokens);
	free_cmd_list(shell->cmds);
	shell->cmds = NULL;
}

void	shell_loop(t_shell *shell)
{
	char	*line;

	while (1)
	{
		line = readline("minishell$ ");
		if (g_signal == SIGINT)
		{
			shell->last_status = 130;
			g_signal = 0;
		}
		if (!line)
		{
			ft_putstr_fd("exit\n", 1);
			break ;
		}
		if (*line)
			add_history(line);
		process_line(line, shell);
		free(line);
	}
	rl_clear_history();
}
