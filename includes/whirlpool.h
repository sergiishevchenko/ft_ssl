#ifndef WHIRLPOOL_H
# define WHIRLPOOL_H

# include <stdint.h>
# include <stddef.h>

typedef struct s_whirlpool_ctx
{
	uint64_t	state[8];
	uint64_t	count;
	uint8_t		buffer[64];
	uint32_t	buflen;
}	t_whirlpool_ctx;

void	whirlpool_init(void *ctx);
void	whirlpool_update(void *ctx, const uint8_t *data, size_t len);
void	whirlpool_final(uint8_t *digest, void *ctx);

#endif
