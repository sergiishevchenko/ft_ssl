#include "md5.h"
#include "ft_ssl.h"

static const uint32_t	g_t[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

static const uint32_t	g_s[64] = {
	7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
	5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
	4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
	6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

static uint32_t	md5_f(uint32_t i, uint32_t b, uint32_t c, uint32_t d)
{
	if (i < 16)
		return ((b & c) | (~b & d));
	if (i < 32)
		return ((d & b) | (~d & c));
	if (i < 48)
		return (b ^ c ^ d);
	return (c ^ (b | ~d));
}

static uint32_t	md5_g(uint32_t i)
{
	if (i < 16)
		return (i);
	if (i < 32)
		return ((5 * i + 1) % 16);
	if (i < 48)
		return ((3 * i + 5) % 16);
	return ((7 * i) % 16);
}

static uint32_t	left_rotate(uint32_t x, uint32_t n)
{
	return ((x << n) | (x >> (32 - n)));
}

static void	md5_transform(t_md5_ctx *ctx, const uint8_t block[64])
{
	uint32_t	m[16];
	uint32_t	a;
	uint32_t	f;
	uint32_t	tmp;
	uint32_t	i;

	i = 0;
	while (i < 16)
	{
		m[i] = (uint32_t)block[i * 4]
			| ((uint32_t)block[i * 4 + 1] << 8)
			| ((uint32_t)block[i * 4 + 2] << 16)
			| ((uint32_t)block[i * 4 + 3] << 24);
		i++;
	}
	a = ctx->state[0];
	f = ctx->state[1];
	tmp = ctx->state[2];
	i = ctx->state[3];
	{
		uint32_t	aa = a;
		uint32_t	bb = f;
		uint32_t	cc = tmp;
		uint32_t	dd = i;
		uint32_t	j = 0;
		uint32_t	fv;
		uint32_t	gv;
		uint32_t	temp;

		while (j < 64)
		{
			fv = md5_f(j, bb, cc, dd);
			gv = md5_g(j);
			temp = dd;
			dd = cc;
			cc = bb;
			bb = bb + left_rotate(aa + fv + g_t[j] + m[gv], g_s[j]);
			aa = temp;
			j++;
		}
		ctx->state[0] += aa;
		ctx->state[1] += bb;
		ctx->state[2] += cc;
		ctx->state[3] += dd;
	}
}

void	md5_init(void *ctx)
{
	t_md5_ctx	*c;

	c = (t_md5_ctx *)ctx;
	c->state[0] = 0x67452301;
	c->state[1] = 0xefcdab89;
	c->state[2] = 0x98badcfe;
	c->state[3] = 0x10325476;
	c->count = 0;
	c->buflen = 0;
}

void	md5_update(void *ctx, const uint8_t *data, size_t len)
{
	t_md5_ctx	*c;
	size_t		i;

	c = (t_md5_ctx *)ctx;
	i = 0;
	while (i < len)
	{
		c->buffer[c->buflen++] = data[i];
		if (c->buflen == 64)
		{
			md5_transform(c, c->buffer);
			c->buflen = 0;
		}
		i++;
	}
	c->count += len;
}

void	md5_final(uint8_t *digest, void *ctx)
{
	t_md5_ctx	*c;
	uint64_t	bits;
	int			i;

	c = (t_md5_ctx *)ctx;
	bits = c->count * 8;
	c->buffer[c->buflen++] = 0x80;
	while (c->buflen != 56)
	{
		if (c->buflen == 64)
		{
			md5_transform(c, c->buffer);
			c->buflen = 0;
		}
		if (c->buflen != 56)
			c->buffer[c->buflen++] = 0x00;
	}
	i = 0;
	while (i < 8)
	{
		c->buffer[56 + i] = (uint8_t)(bits >> (i * 8));
		i++;
	}
	md5_transform(c, c->buffer);
	i = 0;
	while (i < 4)
	{
		digest[i * 4] = (uint8_t)(c->state[i]);
		digest[i * 4 + 1] = (uint8_t)(c->state[i] >> 8);
		digest[i * 4 + 2] = (uint8_t)(c->state[i] >> 16);
		digest[i * 4 + 3] = (uint8_t)(c->state[i] >> 24);
		i++;
	}
}
