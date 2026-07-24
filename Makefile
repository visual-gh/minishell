# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: Visual <github.com/visual-gh>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/05/06 19:05:23 by Visual            #+#    #+#              #
#    Updated: 2026/07/25 01:58:06 by Visual           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = minishell

CC		= cc
CFLAGS	= -Wall -Wextra -Werror

RM		= rm -f

LIBFT	= libft/libft.a
INC		= -Iincludes -Ilibft
LIBS	= $(LIBFT) -lreadline

SRCS	= src/main.c \
		  src/shell_init.c \
		  src/shell_loop.c \
		  src/env/env_get.c \
		  src/env/env_init.c \
		  src/env/env_set.c \
		  src/env/env_unset.c \
		  src/signals/signals.c \
		  src/signals/signals_heredoc.c \
		  src/heredoc/heredoc.c \
		  src/heredoc/heredoc_expand.c \
		  src/lexer/lexer.c \
		  src/lexer/lexer_utils.c \
		  src/lexer/lexer_scan.c \
		  src/lexer/token_init.c \
		  src/parser/parser.c \
		  src/parser/parser_utils.c \
		  src/parser/cmd_init.c \
		  src/expander/expander.c \
		  src/expander/expander_var.c \
		  src/expander/expander_word.c \
		  src/utils/error.c \
		  src/utils/free.c \
		  src/builtins/echo.c \
		  src/builtins/pwd.c \
		  src/builtins/cd.c \
		  src/builtins/export.c \
		  src/builtins/export_print.c \
		  src/builtins/unset.c \
		  src/builtins/env.c \
		  src/builtins/exit.c \
		  src/executor/executor.c \
		  src/executor/pipeline.c \
		  src/executor/pipeline_utils.c \
		  src/executor/redirs.c \
		  src/executor/resolve_path.c \
		  src/executor/run_builtin.c \
		  src/executor/run_child.c \
		  src/executor/single.c

OBJS	= $(SRCS:.c=.o)

all: $(LIBFT) $(NAME)

$(LIBFT):
	$(MAKE) -C libft bonus

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(NAME)

$(OBJS): includes/minishell.h

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	$(MAKE) -C libft clean
	$(RM) $(OBJS)

fclean: clean
	$(MAKE) -C libft fclean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re $(LIBFT)
