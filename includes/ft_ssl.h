#ifndef FT_SSL_H
# define FT_SSL_H

# include <stdint.h>
# include <stddef.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdlib.h>

# define BUF_SIZE 4096

typedef struct s_hash_func
{
	char	*name_upper;
	char	*name_lower;
	int		digest_size;
	int		ctx_size;
	void	(*init)(void *ctx);
	void	(*update)(void *ctx, const uint8_t *data, size_t len);
	void	(*final)(uint8_t *digest, void *ctx);
}	t_hash_func;

typedef struct s_flags
{
	int	p;
	int	q;
	int	r;
}	t_flags;

typedef struct s_ssl
{
	t_hash_func	*hash;
	t_flags		flags;
	int			argc;
	char		**argv;
	int			cmd_index;
}	t_ssl;

t_hash_func		*get_hash_functions(void);
int				get_hash_count(void);
t_hash_func		*find_hash(const char *name);
void			print_usage(void);
void			print_invalid_command(const char *cmd);
int				execute_command(t_ssl *ssl);

void			ft_putstr_fd(const char *s, int fd);
void			ft_putchar_fd(char c, int fd);
void			ft_putendl_fd(const char *s, int fd);
int				ft_strcmp(const char *s1, const char *s2);
size_t			ft_strlen(const char *s);
void			*ft_memcpy(void *dst, const void *src, size_t n);
void			*ft_memset(void *b, int c, size_t len);
void			ft_putnbr_hex(uint8_t byte, int fd);
char			*ft_strdup(const char *s);
char			*ft_strjoin_free(char *s1, char *s2, int free_s1);

uint8_t			*read_fd(int fd, size_t *out_len);
void			digest_to_hex(uint8_t *digest, int size, char *out);
void			hash_data(t_hash_func *h, const uint8_t *data,
					size_t len, uint8_t *digest);

void			process_stdin(t_ssl *ssl);
void			process_string(t_ssl *ssl, const char *str);
void			process_file(t_ssl *ssl, const char *filename);
void			print_hash_result(t_ssl *ssl, const char *label,
					const char *hex, int is_file);

int				interactive_mode(void);

#endif
