# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: Visual <github.com/visual-gh>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/05/06 19:05:23 by Visual            #+#    #+#              #
#    Updated: 2026/05/08 19:40:41 by Visual           ###   ########.fr        #
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
		  src/utils/free.c

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
