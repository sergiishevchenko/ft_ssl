#include "ft_ssl.h"

static char	*get_next_word(char **line)
{
	char	*start;
	char	*word;
	size_t	len;

	while (**line == ' ' || **line == '\t')
		(*line)++;
	if (**line == '\0' || **line == '\n')
		return (NULL);
	if (**line == '"')
	{
		(*line)++;
		start = *line;
		while (**line && **line != '"')
			(*line)++;
		len = *line - start;
		if (**line == '"')
			(*line)++;
		word = malloc(len + 1);
		if (!word)
			return (NULL);
		ft_memcpy(word, start, len);
		word[len] = '\0';
		return (word);
	}
	start = *line;
	while (**line && **line != ' ' && **line != '\t' && **line != '\n')
		(*line)++;
	len = *line - start;
	word = malloc(len + 1);
	if (!word)
		return (NULL);
	ft_memcpy(word, start, len);
	word[len] = '\0';
	return (word);
}

static char	**split_line(char *line, int *argc)
{
	char	**argv;
	char	*ptr;
	char	*word;
	int		i;

	argv = malloc(sizeof(char *) * 256);
	if (!argv)
		return (NULL);
	ptr = line;
	i = 0;
	word = get_next_word(&ptr);
	while (word)
	{
		if (i >= 255)
		{
			free(word);
			break ;
		}
		argv[i++] = word;
		word = get_next_word(&ptr);
	}
	argv[i] = NULL;
	*argc = i;
	if (i == 0)
	{
		free(argv);
		return (NULL);
	}
	return (argv);
}

static void	free_argv(char **argv, int argc)
{
	int	i;

	i = 0;
	while (i < argc)
	{
		free(argv[i]);
		i++;
	}
	free(argv);
}

static char	*grow_line(char *line, size_t len, size_t *cap)
{
	char	*tmp;

	*cap *= 2;
	tmp = malloc(*cap);
	if (!tmp)
	{
		free(line);
		return (NULL);
	}
	ft_memcpy(tmp, line, len);
	free(line);
	return (tmp);
}

static char	*read_line(void)
{
	char	*line;
	char	buf[1];
	size_t	len;
	size_t	cap;
	ssize_t	n;

	cap = 128;
	len = 0;
	line = malloc(cap);
	if (!line)
		return (NULL);
	n = read(0, buf, 1);
	while (n > 0)
	{
		if (len + 1 >= cap)
		{
			line = grow_line(line, len, &cap);
			if (!line)
				return (NULL);
		}
		line[len++] = buf[0];
		if (buf[0] == '\n')
			break ;
		n = read(0, buf, 1);
	}
	if (n <= 0 && len == 0)
	{
		free(line);
		return (NULL);
	}
	line[len] = '\0';
	return (line);
}

static int	exec_interactive_line(char *line)
{
	char		**argv;
	int			argc;
	t_hash_func	*h;
	t_ssl		ssl;

	argv = split_line(line, &argc);
	if (!argv || argc == 0)
	{
		free(argv);
		return (0);
	}
	if (ft_strcmp(argv[0], "exit") == 0
		|| ft_strcmp(argv[0], "quit") == 0)
	{
		free_argv(argv, argc);
		return (1);
	}
	h = find_hash(argv[0]);
	if (!h)
	{
		print_invalid_command(argv[0]);
		free_argv(argv, argc);
		return (0);
	}
	ssl.hash = h;
	ssl.argc = argc;
	ssl.argv = argv;
	ssl.cmd_index = 1;
	execute_command(&ssl);
	free_argv(argv, argc);
	return (0);
}

int	interactive_mode(void)
{
	char	*line;
	int		is_tty;

	is_tty = isatty(0);
	if (is_tty)
		ft_putstr_fd("ft_ssl> ", 1);
	line = read_line();
	while (line)
	{
		if (exec_interactive_line(line))
		{
			free(line);
			break ;
		}
		free(line);
		if (is_tty)
			ft_putstr_fd("ft_ssl> ", 1);
		line = read_line();
	}
	return (0);
}
