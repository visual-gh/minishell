/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_hex.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 09:24:37 by Visual            #+#    #+#             */
/*   Updated: 2025/11/24 13:58:33 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_puthex(unsigned int n, char *base)
{
	int	len;

	len = 0;
	if (n >= 16)
		len += ft_puthex((n / 16), base);
	len += ft_print_char(base[n % 16]);
	return (len);
}

int	ft_print_hex(unsigned int n, int upper)
{
	if (upper)
		return (ft_puthex(n, "0123456789ABCDEF"));
	return (ft_puthex(n, "0123456789abcdef"));
}
