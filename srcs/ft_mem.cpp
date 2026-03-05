#include "ft_mem.hpp"

void	ft_memcpy(void *dest, const void *src, size_t n)
{
	size_t i = 0;
	while (i < n)
		static_cast<char *>(dest)[i] = static_cast<const char *>(src)[i];
}

void	ft_memset(void *s, int c, size_t n)
{
	size_t i = 0;
	while (i < n)
		((char *)s)[i++] = c;
}
