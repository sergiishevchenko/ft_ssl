NAME = ft_ssl

CC = cc
CFLAGS = -Wall -Wextra -Werror
INCLUDES = -I includes

RESET = \033[0m
BOLD = \033[1m
DIM = \033[2m
RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
CYAN = \033[36m
MAGENTA = \033[35m

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
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@printf "$(GREEN)$(BOLD)\n  ✓  $(NAME)$(RESET) $(DIM)ready$(RESET)\n\n"

%.o: %.c $(HEADERS)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	@printf "  $(CYAN)▸$(RESET)  $(DIM)compile$(RESET)  $(YELLOW)$<$(RESET)\n"

clean:
	@rm -f $(OBJS)
	@printf "  $(MAGENTA)✕$(RESET)  $(DIM)removed object files$(RESET)\n"

fclean: clean
	@rm -f $(NAME)
	@printf "  $(RED)✕$(RESET)  $(DIM)removed$(RESET) $(BOLD)$(NAME)$(RESET)\n"

re: fclean all

.PHONY: all clean fclean re
