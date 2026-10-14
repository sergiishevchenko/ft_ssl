#include "ft_ssl.h"
#include <errno.h>

static const char	*io_error_msg(void)
{
	if (errno == ENOENT)
		return ("No such file or directory");
	if (errno == EISDIR)
		return ("Is a directory");
	if (errno == EACCES)
		return ("Permission denied");
	if (errno == ENOMEM)
		return ("Cannot allocate memory");
	return ("Invalid argument");
}

static void	print_io_error(t_ssl *ssl, const char *name)
{
	ft_putstr_fd("ft_ssl: ", 2);
	ft_putstr_fd(ssl->hash->name_lower, 2);
	ft_putstr_fd(": ", 2);
	ft_putstr_fd(name, 2);
	ft_putstr_fd(": ", 2);
	ft_putendl_fd(io_error_msg(), 2);
	ssl->error = 1;
}

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

static void	print_stdin_p(t_ssl *ssl, uint8_t *data,
		size_t len, const char *hex)
{
	if (ssl->flags.q)
	{
		if (data && len > 0)
			write(1, data, len);
		ft_putendl_fd(hex, 1);
		return ;
	}
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

static void	print_stdin_nop(t_ssl *ssl, const char *hex)
{
	if (ssl->flags.q)
	{
		ft_putendl_fd(hex, 1);
		return ;
	}
	if (ssl->flags.r)
	{
		ft_putstr_fd(hex, 1);
		ft_putendl_fd(" *stdin", 1);
		return ;
	}
	ft_putstr_fd("(stdin)= ", 1);
	ft_putendl_fd(hex, 1);
}

void	process_stdin(t_ssl *ssl)
{
	uint8_t	*data;
	size_t	len;
	uint8_t	digest[64];
	char	hex[129];

	data = read_fd(0, &len);
	if (len == (size_t)-1)
	{
		print_io_error(ssl, "stdin");
		return ;
	}
	if (hash_data(ssl->hash, data, len, digest))
	{
		print_io_error(ssl, "stdin");
		free(data);
		return ;
	}
	digest_to_hex(digest, ssl->hash->digest_size, hex);
	if (ssl->flags.p)
		print_stdin_p(ssl, data, len, hex);
	else
		print_stdin_nop(ssl, hex);
	free(data);
}

void	process_string(t_ssl *ssl, const char *str)
{
	uint8_t	digest[64];
	char	hex[129];

	if (hash_data(ssl->hash, (const uint8_t *)str, ft_strlen(str), digest))
	{
		print_io_error(ssl, str);
		return ;
	}
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
		print_io_error(ssl, filename);
		return ;
	}
	data = read_fd(fd, &len);
	if (len == (size_t)-1)
	{
		print_io_error(ssl, filename);
		close(fd);
		return ;
	}
	close(fd);
	if (hash_data(ssl->hash, data, len, digest))
	{
		print_io_error(ssl, filename);
		free(data);
		return ;
	}
	digest_to_hex(digest, ssl->hash->digest_size, hex);
	print_hash_result(ssl, filename, hex, 1);
	free(data);
}
