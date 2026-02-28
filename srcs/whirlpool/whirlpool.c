#include "whirlpool.h"
#include "ft_ssl.h"

extern const uint64_t	g_whirlpool_c0[256];
extern const uint64_t	g_whirlpool_rc[10];

static uint64_t	wp_round(const uint64_t *state, int row)
{
	uint64_t	result;
	uint64_t	c;
	int			t;
	uint8_t		idx;

	result = 0;
	t = 0;
	while (t < 8)
	{
		idx = (uint8_t)(state[(row - t + 8) % 8] >> (56 - t * 8));
		c = g_whirlpool_c0[idx];
		if (t > 0)
			c = (c >> (t * 8)) | (c << (64 - t * 8));
		result ^= c;
		t++;
	}
	return (result);
}

static void	wp_load_block(uint64_t *dst, const uint8_t *block)
{
	int	i;

	i = 0;
	while (i < 8)
	{
		dst[i] = ((uint64_t)block[i * 8] << 56)
			| ((uint64_t)block[i * 8 + 1] << 48)
			| ((uint64_t)block[i * 8 + 2] << 40)
			| ((uint64_t)block[i * 8 + 3] << 32)
			| ((uint64_t)block[i * 8 + 4] << 24)
			| ((uint64_t)block[i * 8 + 5] << 16)
			| ((uint64_t)block[i * 8 + 6] << 8)
			| ((uint64_t)block[i * 8 + 7]);
		i++;
	}
}

static void	whirlpool_transform(t_whirlpool_ctx *ctx, const uint8_t block[64])
{
	uint64_t	k[8];
	uint64_t	s[8];
	uint64_t	nk[8];
	uint64_t	ns[8];
	uint64_t	blk[8];
	int			i;
	int			r;

	wp_load_block(blk, block);
	i = -1;
	while (++i < 8)
	{
		k[i] = ctx->state[i];
		s[i] = blk[i] ^ k[i];
	}
	r = 0;
	while (r < 10)
	{
		i = -1;
		while (++i < 8)
			nk[i] = wp_round(k, i);
		nk[0] ^= g_whirlpool_rc[r];
		i = -1;
		while (++i < 8)
			k[i] = nk[i];
		i = -1;
		while (++i < 8)
			ns[i] = wp_round(s, i) ^ k[i];
		i = -1;
		while (++i < 8)
			s[i] = ns[i];
		r++;
	}
	i = -1;
	while (++i < 8)
		ctx->state[i] ^= s[i] ^ blk[i];
}

void	whirlpool_init(void *ctx)
{
	t_whirlpool_ctx	*c;

	c = (t_whirlpool_ctx *)ctx;
	ft_memset(c, 0, sizeof(t_whirlpool_ctx));
}

void	whirlpool_update(void *ctx, const uint8_t *data, size_t len)
{
	t_whirlpool_ctx	*c;
	size_t			i;

	c = (t_whirlpool_ctx *)ctx;
	i = 0;
	while (i < len)
	{
		c->buffer[c->buflen++] = data[i];
		if (c->buflen == 64)
		{
			whirlpool_transform(c, c->buffer);
			c->buflen = 0;
		}
		i++;
	}
	c->count += len;
}

void	whirlpool_final(uint8_t *digest, void *ctx)
{
	t_whirlpool_ctx	*c;
	uint64_t		bits;
	int				i;

	c = (t_whirlpool_ctx *)ctx;
	bits = c->count * 8;
	c->buffer[c->buflen++] = 0x80;
	if (c->buflen > 32)
	{
		while (c->buflen < 64)
			c->buffer[c->buflen++] = 0x00;
		whirlpool_transform(c, c->buffer);
		c->buflen = 0;
	}
	while (c->buflen < 32)
		c->buffer[c->buflen++] = 0x00;
	ft_memset(c->buffer + 32, 0, 24);
	i = 7;
	while (i >= 0)
	{
		c->buffer[56 + (7 - i)] = (uint8_t)(bits >> (i * 8));
		i--;
	}
	whirlpool_transform(c, c->buffer);
	i = 0;
	while (i < 8)
	{
		digest[i * 8] = (uint8_t)(c->state[i] >> 56);
		digest[i * 8 + 1] = (uint8_t)(c->state[i] >> 48);
		digest[i * 8 + 2] = (uint8_t)(c->state[i] >> 40);
		digest[i * 8 + 3] = (uint8_t)(c->state[i] >> 32);
		digest[i * 8 + 4] = (uint8_t)(c->state[i] >> 24);
		digest[i * 8 + 5] = (uint8_t)(c->state[i] >> 16);
		digest[i * 8 + 6] = (uint8_t)(c->state[i] >> 8);
		digest[i * 8 + 7] = (uint8_t)(c->state[i]);
		i++;
	}
}
