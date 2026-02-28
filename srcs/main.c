#include "ft_ssl.h"

int	main(int argc, char **argv)
{
	t_hash_func	*h;
	t_ssl		ssl;

	if (argc < 2)
	{
		print_usage();
		return (interactive_mode());
	}
	h = find_hash(argv[1]);
	if (!h)
	{
		print_invalid_command(argv[1]);
		return (1);
	}
	ssl.hash = h;
	ssl.argc = argc;
	ssl.argv = argv;
	ssl.cmd_index = 2;
	return (execute_command(&ssl));
}
