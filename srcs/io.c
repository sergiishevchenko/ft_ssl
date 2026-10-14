#include "ft_ssl.h"

uint8_t	*read_fd(int fd, size_t *out_len)
{
	uint8_t	buf[BUF_SIZE];
	uint8_t	*data;
	uint8_t	*tmp;
	ssize_t	n;
	size_t	total;
	size_t	cap;

	data = NULL;
	total = 0;
	cap = 0;
	n = read(fd, buf, BUF_SIZE);
	while (n > 0)
	{
		if ((size_t)n > (size_t)-1 - total)
		{
			free(data);
			*out_len = (size_t)-1;
			return (NULL);
		}
		if (total + (size_t)n > cap)
		{
			if (cap == 0)
				cap = BUF_SIZE;
			while (total + (size_t)n > cap)
			{
				if (cap > ((size_t)-1) / 2)
				{
					free(data);
					*out_len = (size_t)-1;
					return (NULL);
				}
				cap *= 2;
			}
			tmp = malloc(cap);
			if (!tmp)
			{
				free(data);
				*out_len = (size_t)-1;
				return (NULL);
			}
			if (data)
				ft_memcpy(tmp, data, total);
			free(data);
			data = tmp;
		}
		ft_memcpy(data + total, buf, n);
		total += n;
		n = read(fd, buf, BUF_SIZE);
	}
	if (n < 0)
	{
		free(data);
		*out_len = (size_t)-1;
		return (NULL);
	}
	*out_len = total;
	return (data);
}

void	digest_to_hex(uint8_t *digest, int size, char *out)
{
	static const char	hex[] = "0123456789abcdef";
	int					i;

	i = 0;
	while (i < size)
	{
		out[i * 2] = hex[(digest[i] >> 4) & 0x0f];
		out[i * 2 + 1] = hex[digest[i] & 0x0f];
		i++;
	}
	out[size * 2] = '\0';
}

int	hash_data(t_hash_func *h, const uint8_t *data,
		size_t len, uint8_t *digest)
{
	void	*ctx;

	ctx = malloc(h->ctx_size);
	if (!ctx)
		return (1);
	h->init(ctx);
	if (len > 0 && data)
		h->update(ctx, data, len);
	h->final(digest, ctx);
	free(ctx);
	return (0);
}
