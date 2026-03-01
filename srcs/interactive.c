#include "ft_ssl.h"

static int	count_words(char *line)
{
	int	count;
	int	in_word;

	count = 0;
	in_word = 0;
	while (*line)
	{
		if (*line == ' ' || *line == '\t' || *line == '\n')
			in_word = 0;
		else if (!in_word)
		{
			in_word = 1;
			count++;
		}
		line++;
	}
	return (count);
}

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
	int		wc;
	int		i;

	wc = count_words(line);
	if (wc == 0)
	{
		*argc = 0;
		return (NULL);
	}
	argv = malloc(sizeof(char *) * (wc + 1));
	if (!argv)
		return (NULL);
	ptr = line;
	i = 0;
	while (i < wc)
	{
		argv[i] = get_next_word(&ptr);
		i++;
	}
	argv[i] = NULL;
	*argc = wc;
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
			cap *= 2;
			line = ft_strjoin_free(line, "", 1);
			if (!line)
				return (NULL);
			free(line);
			line = malloc(cap);
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

static void	exec_interactive_line(char *line)
{
	char		**argv;
	int			argc;
	t_hash_func	*h;
	t_ssl		ssl;

	argv = split_line(line, &argc);
	if (!argv || argc == 0)
	{
		free(argv);
		return ;
	}
	h = find_hash(argv[0]);
	if (!h)
	{
		ft_putstr_fd("ft_ssl: Error: '", 2);
		ft_putstr_fd(argv[0], 2);
		ft_putendl_fd("' is an invalid command.", 2);
		free_argv(argv, argc);
		return ;
	}
	ssl.hash = h;
	ssl.argc = argc;
	ssl.argv = argv;
	ssl.cmd_index = 1;
	execute_command(&ssl);
	free_argv(argv, argc);
}

int	interactive_mode(void)
{
	char	*line;

	ft_putstr_fd("ft_ssl> ", 1);
	line = read_line();
	while (line)
	{
		exec_interactive_line(line);
		free(line);
		ft_putstr_fd("ft_ssl> ", 1);
		line = read_line();
	}
	return (0);
}
