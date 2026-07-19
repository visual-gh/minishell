# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: Visual <github.com/visual-gh>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/05/06 19:05:23 by Visual            #+#    #+#              #
#    Updated: 2026/07/19 15:34:23 by Visual           ###   ########.fr        #
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
		  src/utils/error.c \
		  src/utils/free.c \
		  src/builtins/echo.c \
		  src/builtins/pwd.c \
		  src/builtins/cd.c \
		  src/builtins/export.c \
		  src/builtins/export_print.c \
		  src/builtins/unset.c \
		  src/builtins/env.c \
		  src/builtins/exit.c

OBJS	= $(SRCS:.c=.o)

all: $(LIBFT) $(NAME)

$(LIBFT):
	$(MAKE) -C libft bonus

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	$(MAKE) -C libft clean
	$(RM) $(OBJS)

fclean: clean
	$(MAKE) -C libft fclean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
