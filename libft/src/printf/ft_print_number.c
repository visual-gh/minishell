/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_number.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 09:25:48 by Visual            #+#    #+#             */
/*   Updated: 2025/11/24 15:48:46 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_putnbr(long nbr)
{
	int	len;

	len = 0;
	if (nbr >= 10)
		len += ft_putnbr(nbr / 10);
	len += ft_print_char((nbr % 10) + '0');
	return (len);
}

int	ft_print_number(int n)
{
	long	nbr;
	int		len;

	nbr = (long)n;
	len = 0;
	if (nbr < 0)
	{
		len += ft_print_char('-');
		nbr *= -1;
	}
	len += ft_putnbr(nbr);
	return (len);
}
