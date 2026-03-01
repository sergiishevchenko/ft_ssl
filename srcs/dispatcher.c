#include "ft_ssl.h"
#include "md5.h"
#include "sha256.h"
#include "whirlpool.h"

static t_hash_func	g_hashes[] = {
	{"MD5", "md5", 16, sizeof(t_md5_ctx),
		md5_init, md5_update, md5_final},
	{"SHA256", "sha256", 32, sizeof(t_sha256_ctx),
		sha256_init, sha256_update, sha256_final},
	{"WHIRLPOOL", "whirlpool", 64, sizeof(t_whirlpool_ctx),
		whirlpool_init, whirlpool_update, whirlpool_final},
	{NULL, NULL, 0, 0, NULL, NULL, NULL}
};

t_hash_func	*get_hash_functions(void)
{
	return (g_hashes);
}

int	get_hash_count(void)
{
	int	i;

	i = 0;
	while (g_hashes[i].name_upper)
		i++;
	return (i);
}

t_hash_func	*find_hash(const char *name)
{
	int	i;

	i = 0;
	while (g_hashes[i].name_lower)
	{
		if (ft_strcmp(g_hashes[i].name_lower, name) == 0)
			return (&g_hashes[i]);
		i++;
	}
	return (NULL);
}

void	print_usage(void)
{
	ft_putendl_fd("usage: ft_ssl command [flags] [file/string]", 2);
}

void	print_invalid_command(const char *cmd)
{
	int	i;

	ft_putstr_fd("ft_ssl: Error: '", 2);
	ft_putstr_fd(cmd, 2);
	ft_putendl_fd("' is an invalid command.", 2);
	ft_putendl_fd("", 2);
	ft_putendl_fd("Commands:", 2);
	i = 0;
	while (g_hashes[i].name_lower)
	{
		ft_putendl_fd(g_hashes[i].name_lower, 2);
		i++;
	}
	ft_putendl_fd("", 2);
	ft_putendl_fd("Flags:", 2);
	ft_putendl_fd("-p -q -r -s", 2);
}
