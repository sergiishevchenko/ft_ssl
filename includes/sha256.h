#ifndef SHA256_H
# define SHA256_H

# include <stdint.h>
# include <stddef.h>

typedef struct s_sha256_ctx
{
	uint32_t	state[8];
	uint64_t	count;
	uint8_t		buffer[64];
	uint32_t	buflen;
}	t_sha256_ctx;

void	sha256_init(void *ctx);
void	sha256_update(void *ctx, const uint8_t *data, size_t len);
void	sha256_final(uint8_t *digest, void *ctx);

#endif
