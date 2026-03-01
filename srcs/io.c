#include "ft_ssl.h"

uint8_t	*read_fd(int fd, size_t *out_len)
{
	uint8_t	buf[BUF_SIZE];
	uint8_t	*data;
	uint8_t	*tmp;
	ssize_t	n;
	size_t	total;

	data = NULL;
	total = 0;
	n = read(fd, buf, BUF_SIZE);
	while (n > 0)
	{
		tmp = malloc(total + n);
		if (!tmp)
		{
			free(data);
			return (NULL);
		}
		if (data)
			ft_memcpy(tmp, data, total);
		ft_memcpy(tmp + total, buf, n);
		free(data);
		data = tmp;
		total += n;
		n = read(fd, buf, BUF_SIZE);
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

void	hash_data(t_hash_func *h, const uint8_t *data,
		size_t len, uint8_t *digest)
{
	uint8_t	ctx_buf[256];

	h->init(ctx_buf);
	h->update(ctx_buf, data, len);
	h->final(digest, ctx_buf);
}
