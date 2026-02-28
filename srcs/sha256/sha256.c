#include "sha256.h"
#include "ft_ssl.h"

static const uint32_t	g_k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static uint32_t	rr(uint32_t x, uint32_t n)
{
	return ((x >> n) | (x << (32 - n)));
}

static void	sha256_schedule(uint32_t w[64], const uint8_t block[64])
{
	uint32_t	i;
	uint32_t	s0;
	uint32_t	s1;

	i = 0;
	while (i < 16)
	{
		w[i] = ((uint32_t)block[i * 4] << 24)
			| ((uint32_t)block[i * 4 + 1] << 16)
			| ((uint32_t)block[i * 4 + 2] << 8)
			| ((uint32_t)block[i * 4 + 3]);
		i++;
	}
	while (i < 64)
	{
		s0 = rr(w[i - 15], 7) ^ rr(w[i - 15], 18) ^ (w[i - 15] >> 3);
		s1 = rr(w[i - 2], 17) ^ rr(w[i - 2], 19) ^ (w[i - 2] >> 10);
		w[i] = w[i - 16] + s0 + w[i - 7] + s1;
		i++;
	}
}

static void	sha256_transform(t_sha256_ctx *ctx, const uint8_t block[64])
{
	uint32_t	w[64];
	uint32_t	v[8];
	uint32_t	i;
	uint32_t	t1;
	uint32_t	t2;

	sha256_schedule(w, block);
	i = 0;
	while (i < 8)
	{
		v[i] = ctx->state[i];
		i++;
	}
	i = 0;
	while (i < 64)
	{
		t1 = v[7] + (rr(v[4], 6) ^ rr(v[4], 11) ^ rr(v[4], 25))
			+ ((v[4] & v[5]) ^ (~v[4] & v[6])) + g_k[i] + w[i];
		t2 = (rr(v[0], 2) ^ rr(v[0], 13) ^ rr(v[0], 22))
			+ ((v[0] & v[1]) ^ (v[0] & v[2]) ^ (v[1] & v[2]));
		v[7] = v[6];
		v[6] = v[5];
		v[5] = v[4];
		v[4] = v[3] + t1;
		v[3] = v[2];
		v[2] = v[1];
		v[1] = v[0];
		v[0] = t1 + t2;
		i++;
	}
	i = 0;
	while (i < 8)
	{
		ctx->state[i] += v[i];
		i++;
	}
}

void	sha256_init(void *ctx)
{
	t_sha256_ctx	*c;

	c = (t_sha256_ctx *)ctx;
	c->state[0] = 0x6a09e667;
	c->state[1] = 0xbb67ae85;
	c->state[2] = 0x3c6ef372;
	c->state[3] = 0xa54ff53a;
	c->state[4] = 0x510e527f;
	c->state[5] = 0x9b05688c;
	c->state[6] = 0x1f83d9ab;
	c->state[7] = 0x5be0cd19;
	c->count = 0;
	c->buflen = 0;
}

void	sha256_update(void *ctx, const uint8_t *data, size_t len)
{
	t_sha256_ctx	*c;
	size_t			i;

	c = (t_sha256_ctx *)ctx;
	i = 0;
	while (i < len)
	{
		c->buffer[c->buflen++] = data[i];
		if (c->buflen == 64)
		{
			sha256_transform(c, c->buffer);
			c->buflen = 0;
		}
		i++;
	}
	c->count += len;
}

void	sha256_final(uint8_t *digest, void *ctx)
{
	t_sha256_ctx	*c;
	uint64_t		bits;
	int				i;

	c = (t_sha256_ctx *)ctx;
	bits = c->count * 8;
	c->buffer[c->buflen++] = 0x80;
	while (c->buflen != 56)
	{
		if (c->buflen == 64)
		{
			sha256_transform(c, c->buffer);
			c->buflen = 0;
		}
		if (c->buflen != 56)
			c->buffer[c->buflen++] = 0x00;
	}
	i = 7;
	while (i >= 0)
	{
		c->buffer[56 + (7 - i)] = (uint8_t)(bits >> (i * 8));
		i--;
	}
	sha256_transform(c, c->buffer);
	i = 0;
	while (i < 8)
	{
		digest[i * 4] = (uint8_t)(c->state[i] >> 24);
		digest[i * 4 + 1] = (uint8_t)(c->state[i] >> 16);
		digest[i * 4 + 2] = (uint8_t)(c->state[i] >> 8);
		digest[i * 4 + 3] = (uint8_t)(c->state[i]);
		i++;
	}
}
