/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_pointer.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 09:27:23 by Visual            #+#    #+#             */
/*   Updated: 2025/11/24 15:48:42 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_putptr(unsigned long long n)
{
	char	*base;
	int		len;

	base = "0123456789abcdef";
	len = 0;
	if (n >= 16)
		len += ft_putptr(n / 16);
	len += ft_print_char(base[n % 16]);
	return (len);
}

int	ft_print_pointer(void *p)
{
	unsigned long long	addr;
	int					len;

	if (!p)
		return (ft_print_string("(nil)"));
	addr = (unsigned long long)p;
	len = 0;
	len += write(1, "0x", 2);
	len += ft_putptr(addr);
	return (len);
}
