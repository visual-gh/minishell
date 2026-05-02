/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_parse.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 09:29:41 by Visual            #+#    #+#             */
/*   Updated: 2025/11/24 13:23:01 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_dispatch(char c, va_list args)
{
	if (c == 'c')
		return (ft_print_char((char)va_arg(args, int)));
	if (c == 's')
		return (ft_print_string(va_arg(args, char *)));
	if (c == 'p')
		return (ft_print_pointer(va_arg(args, void *)));
	if (c == 'd' || c == 'i')
		return (ft_print_number(va_arg(args, int)));
	if (c == 'u')
		return (ft_print_unsigned(va_arg(args, unsigned int)));
	if (c == 'x')
		return (ft_print_hex(va_arg(args, unsigned int), 0));
	if (c == 'X')
		return (ft_print_hex(va_arg(args, unsigned int), 1));
	if (c == '%')
		return (ft_print_char('%'));
	return (0);
}

int	ft_parse(const char *str, va_list args)
{
	int	i;
	int	len;

	i = 0;
	len = 0;
	while (str[i])
	{
		if (str[i] == '%' && str[i + 1])
		{
			len += ft_dispatch(str[i + 1], args);
			i += 2;
		}
		else
			len += ft_print_char(str[i++]);
	}
	return (len);
}
