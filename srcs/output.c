#include "ft_ssl.h"

void	print_hash_result(t_ssl *ssl, const char *label,
		const char *hex, int is_file)
{
	if (ssl->flags.q)
	{
		ft_putendl_fd(hex, 1);
		return ;
	}
	if (ssl->flags.r)
	{
		ft_putstr_fd(hex, 1);
		ft_putstr_fd(" ", 1);
		if (!is_file)
		{
			ft_putstr_fd("\"", 1);
			ft_putstr_fd(label, 1);
			ft_putstr_fd("\"", 1);
		}
		else
			ft_putstr_fd(label, 1);
		ft_putstr_fd("\n", 1);
		return ;
	}
	ft_putstr_fd(ssl->hash->name_upper, 1);
	ft_putstr_fd(" (", 1);
	if (!is_file)
	{
		ft_putstr_fd("\"", 1);
		ft_putstr_fd(label, 1);
		ft_putstr_fd("\"", 1);
	}
	else
		ft_putstr_fd(label, 1);
	ft_putstr_fd(") = ", 1);
	ft_putendl_fd(hex, 1);
}

void	process_stdin(t_ssl *ssl)
{
	uint8_t	*data;
	size_t	len;
	uint8_t	digest[64];
	char	hex[129];

	data = read_fd(0, &len);
	if (ssl->flags.p)
	{
		if (data && len > 0)
			write(1, data, len);
	}
	hash_data(ssl->hash, data, len, digest);
	digest_to_hex(digest, ssl->hash->digest_size, hex);
	if (ssl->flags.p && !ssl->flags.q)
	{
		ft_putstr_fd("(\"", 1);
		if (data && len > 0)
		{
			if (data[len - 1] == '\n')
				write(1, data, len - 1);
			else
				write(1, data, len);
		}
		ft_putstr_fd("\")= ", 1);
		ft_putendl_fd(hex, 1);
	}
	else if (ssl->flags.q)
	{
		ft_putendl_fd(hex, 1);
	}
	else if (ssl->flags.r)
	{
		ft_putstr_fd(hex, 1);
		ft_putendl_fd(" *stdin", 1);
	}
	else
	{
		ft_putstr_fd("(stdin)= ", 1);
		ft_putendl_fd(hex, 1);
	}
	free(data);
}

void	process_string(t_ssl *ssl, const char *str)
{
	uint8_t	digest[64];
	char	hex[129];

	hash_data(ssl->hash, (const uint8_t *)str, ft_strlen(str), digest);
	digest_to_hex(digest, ssl->hash->digest_size, hex);
	print_hash_result(ssl, str, hex, 0);
}

void	process_file(t_ssl *ssl, const char *filename)
{
	int		fd;
	uint8_t	*data;
	size_t	len;
	uint8_t	digest[64];
	char	hex[129];

	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		ft_putstr_fd("ft_ssl: ", 2);
		ft_putstr_fd(ssl->hash->name_lower, 2);
		ft_putstr_fd(": ", 2);
		ft_putstr_fd(filename, 2);
		ft_putendl_fd(": No such file or directory", 2);
		return ;
	}
	data = read_fd(fd, &len);
	close(fd);
	hash_data(ssl->hash, data, len, digest);
	digest_to_hex(digest, ssl->hash->digest_size, hex);
	print_hash_result(ssl, filename, hex, 1);
	free(data);
}
