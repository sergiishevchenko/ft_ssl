#ifndef MD5_H
# define MD5_H

# include <stdint.h>
# include <stddef.h>

typedef struct s_md5_ctx
{
	uint32_t	state[4];
	uint64_t	count;
	uint8_t		buffer[64];
	uint32_t	buflen;
}	t_md5_ctx;

void	md5_init(void *ctx);
void	md5_update(void *ctx, const uint8_t *data, size_t len);
void	md5_final(uint8_t *digest, void *ctx);

#endif
