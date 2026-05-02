# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: Visual <github.com/visual-gh>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/10 14:22:43 by Visual            #+#    #+#              #
#    Updated: 2026/02/10 14:50:59 by Visual           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= libft.a

CC			= cc
CFLAGS		= -Wall -Wextra -Werror
INCLUDES	= -I.
AR			= ar rcs
RM			= rm -f

# BUFFER_SIZE configuration (default: 42, override with: make BUFFER_SIZE=1024)
BUFFER_SIZE	?= 42
CFLAGS		+= -D BUFFER_SIZE=$(BUFFER_SIZE)

# ========== SOURCE FILES ==========

# Libc Functions (23 files)
SRCS_LIBC	= src/libc/ft_atoi.c \
			  src/libc/ft_bzero.c \
			  src/libc/ft_calloc.c \
			  src/libc/ft_isalnum.c \
			  src/libc/ft_isalpha.c \
			  src/libc/ft_isascii.c \
			  src/libc/ft_isdigit.c \
			  src/libc/ft_isprint.c \
			  src/libc/ft_memchr.c \
			  src/libc/ft_memcmp.c \
			  src/libc/ft_memcpy.c \
			  src/libc/ft_memmove.c \
			  src/libc/ft_memset.c \
			  src/libc/ft_strchr.c \
			  src/libc/ft_strdup.c \
			  src/libc/ft_strlcat.c \
			  src/libc/ft_strlcpy.c \
			  src/libc/ft_strlen.c \
			  src/libc/ft_strncmp.c \
			  src/libc/ft_strnstr.c \
			  src/libc/ft_strrchr.c \
			  src/libc/ft_tolower.c \
			  src/libc/ft_toupper.c

# Additional Functions (11 files)
SRCS_ADD	= src/additional/ft_itoa.c \
			  src/additional/ft_substr.c \
			  src/additional/ft_strjoin.c \
			  src/additional/ft_strtrim.c \
			  src/additional/ft_split.c \
			  src/additional/ft_strmapi.c \
			  src/additional/ft_striteri.c \
			  src/additional/ft_putchar_fd.c \
			  src/additional/ft_putstr_fd.c \
			  src/additional/ft_putendl_fd.c \
			  src/additional/ft_putnbr_fd.c

# Printf Module (8 files)
SRCS_PRINTF	= src/printf/ft_printf.c \
			  src/printf/ft_parse.c \
			  src/printf/ft_print_char.c \
			  src/printf/ft_print_string.c \
			  src/printf/ft_print_pointer.c \
			  src/printf/ft_print_number.c \
			  src/printf/ft_print_unsigned.c \
			  src/printf/ft_print_hex.c

# Get Next Line Module (1 file)
SRCS_GNL	= src/gnl/get_next_line.c

# Bonus Functions (9 files)
SRCS_BONUS	= src/bonus/ft_lstnew_bonus.c \
			  src/bonus/ft_lstadd_front_bonus.c \
			  src/bonus/ft_lstsize_bonus.c \
			  src/bonus/ft_lstlast_bonus.c \
			  src/bonus/ft_lstadd_back_bonus.c \
			  src/bonus/ft_lstdelone_bonus.c \
			  src/bonus/ft_lstclear_bonus.c \
			  src/bonus/ft_lstiter_bonus.c \
			  src/bonus/ft_lstmap_bonus.c

# Combine all mandatory sources
SRCS		= $(SRCS_LIBC) $(SRCS_ADD) $(SRCS_PRINTF) $(SRCS_GNL)

# Object files
OBJS		= $(SRCS:.c=.o)
BONUS_OBJS	= $(SRCS_BONUS:.c=.o)

# ========== RULES ==========

all: $(NAME)

$(NAME): $(OBJS)
	@$(AR) $(NAME) $(OBJS)
	@echo "✓ libft.a created successfully (43 modules)"

bonus: $(OBJS) $(BONUS_OBJS)
	@$(AR) $(NAME) $(OBJS) $(BONUS_OBJS)
	@echo "✓ libft.a created with bonus (52 modules)"

%.o: %.c
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	@echo "Compiled: $<"

clean:
	@$(RM) $(OBJS) $(BONUS_OBJS)
	@echo "✓ Object files cleaned"

fclean: clean
	@$(RM) $(NAME)
	@echo "✓ libft.a removed"

re: fclean all

.PHONY: all bonus clean fclean re
