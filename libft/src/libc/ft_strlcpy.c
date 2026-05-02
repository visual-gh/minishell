/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcpy.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Visual <github.com/visual-gh>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/02 16:18:52 by Visual            #+#    #+#             */
/*   Updated: 2025/10/03 18:02:49 by Visual           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlcpy(char *dst, const char *src, size_t size)
{
	size_t	len;

	len = ft_strlen(src);
	if (size)
	{
		if (len >= size)
		{
			ft_memcpy(dst, src, size - 1);
			dst[size - 1] = '\0';
		}
		else
			ft_memcpy(dst, src, len + 1);
	}
	return (len);
}
