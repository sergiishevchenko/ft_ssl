#include "ft_ssl.h"

#define MAX_STRINGS 256

static void	parse_flags(t_ssl *ssl, int *i,
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
			if (*i + 1 < ssl->argc)
			{
				(*i)++;
				strings[*str_count] = ssl->argv[*i];
				(*str_count)++;
			}
		}
		else
			break ;
		(*i)++;
	}
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
	had_target = 0;
	parse_flags(ssl, &i, strings, &str_count);
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
	return (0);
}
