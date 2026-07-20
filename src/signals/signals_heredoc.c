/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals_heredoc.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 16:25:43 by Visual            #+#    #+#             */
/*   Updated: 2026/07/20 17:08:01 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	sigint_heredoc(int sig)
{
	g_signal = sig;
	close(STDIN_FILENO);
}

void	signals_heredoc(void)
{
	signal(SIGINT, sigint_heredoc);
	signal(SIGQUIT, SIG_IGN);
}
