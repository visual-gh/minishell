/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shell_loop.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 18:56:11 by Visual            #+#    #+#             */
/*   Updated: 2026/08/04 19:07:50 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_blank(char *line)
{
	while (*line == ' ' || *line == '\t')
		line++;
	return (*line == '\0');
}

static int	ready_to_execute(t_shell *shell)
{
	return (shell->cmds && read_heredocs(shell) == 0
		&& expand_cmds(shell) == 0);
}

static void	process_line(t_shell *shell)
{
	if (is_blank(shell->line))
		return ;
	shell->tokens = lexer(shell->line);
	if (!shell->tokens)
	{
		shell->last_status = 2;
		return ;
	}
	if (parse(shell->tokens, shell) == -1)
		shell->last_status = 2;
	else if (ready_to_execute(shell))
		shell->last_status = execute(shell);
	free_tokens(shell->tokens);
	shell->tokens = NULL;
	free_cmd_list(shell->cmds);
	shell->cmds = NULL;
}

void	shell_loop(t_shell *shell)
{
	while (1)
	{
		shell->line = readline("minishell$ ");
		if (g_signal == SIGINT)
		{
			shell->last_status = 130;
			g_signal = 0;
		}
		if (!shell->line)
		{
			ft_putstr_fd("exit\n", 1);
			break ;
		}
		if (*shell->line)
			add_history(shell->line);
		process_line(shell);
		free(shell->line);
		shell->line = NULL;
	}
	rl_clear_history();
}
