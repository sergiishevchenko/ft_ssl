#include "ft_ssl.h"

#define MAX_STRINGS 256

static int	parse_flags(t_ssl *ssl, int *i,
		const char **strings, int *str_count)
{
	*str_count = 0;
	while (*i < ssl->argc)
	{
		if (ft_strcmp(ssl->argv[*i], "-p") == 0)
			ssl->flags.p = 1;
		else if (ft_strcmp(ssl->argv[*i], "-q") == 0)
			ssl->flags.q = 1;
		else if (ft_strcmp(ssl->argv[*i], "-r") == 0)
			ssl->flags.r = 1;
		else if (ft_strcmp(ssl->argv[*i], "-s") == 0)
		{
			if (*i + 1 >= ssl->argc)
			{
				ft_putstr_fd("ft_ssl: ", 2);
				ft_putstr_fd(ssl->hash->name_lower, 2);
				ft_putendl_fd(": option requires an argument -- s", 2);
				ssl->error = 1;
				return (1);
			}
			(*i)++;
			if (*str_count < MAX_STRINGS)
			{
				strings[*str_count] = ssl->argv[*i];
				(*str_count)++;
			}
		}
		else
			break ;
		(*i)++;
	}
	return (0);
}

int	execute_command(t_ssl *ssl)
{
	int			i;
	int			had_target;
	const char	*strings[MAX_STRINGS];
	int			str_count;
	int			j;

	i = ssl->cmd_index;
	ssl->flags.p = 0;
	ssl->flags.q = 0;
	ssl->flags.r = 0;
	ssl->error = 0;
	had_target = 0;
	if (parse_flags(ssl, &i, strings, &str_count))
		return (1);
	if (ssl->flags.p)
		process_stdin(ssl);
	j = 0;
	while (j < str_count)
	{
		had_target = 1;
		process_string(ssl, strings[j]);
		j++;
	}
	while (i < ssl->argc)
	{
		had_target = 1;
		process_file(ssl, ssl->argv[i]);
		i++;
	}
	if (!had_target && !ssl->flags.p)
		process_stdin(ssl);
	if (ssl->error)
		return (1);
	return (0);
}
