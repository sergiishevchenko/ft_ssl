NAME = ft_ssl

CC = cc
CFLAGS = -Wall -Wextra -Werror
INCLUDES = -I includes

SRCS = srcs/main.c \
       srcs/utils.c \
       srcs/io.c \
       srcs/output.c \
       srcs/dispatcher.c \
       srcs/execute.c \
       srcs/interactive.c \
       srcs/md5/md5.c \
       srcs/sha256/sha256.c \
       srcs/whirlpool/whirlpool.c \
       srcs/whirlpool/whirlpool_tables.c

OBJS = $(SRCS:.c=.o)
HEADERS = includes/ft_ssl.h includes/md5.h includes/sha256.h includes/whirlpool.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
