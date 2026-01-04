NAME = itzstd

SRCS_DIR = src
HEADERS_DIR = includes
BUILD_DIR = build

SRCS = main.c

SRCS := $(addprefix $(SRCS_DIR)/, $(SRCS))

OBJS := $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS:.c=.o)))

DEP=$(OBJS:.o=.d)

CC = gcc

CFLAGS = -Wall -Wextra -Werror -I$(HEADERS_DIR)

LDFLAGS = -lm -lzstd

$(NAME): $(OBJS)
	@ printf " \033[33m  Compiling itzstd...\033[m\n"
	@ $(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LDFLAGS)
	@ tput cuu1 && tput el
	@ printf " \033[32m \033[1mItzstd\033[22m compiled\033[m\n"

$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.c
	@ printf " \033[33m  Compiling $(notdir $@)...\033[m\n"
	@ mkdir -p $(BUILD_DIR)
	@ $(CC) $(CFLAGS) -c $< -o $@
	@ tput cuu1 && tput el

all: $(NAME)

clean:
	@ rm -f $(OBJS) $(DEP)
	@ rm -rfd $(BUILD_DIR)
	@ printf " \033[32m Object files cleaned\033[m\n"

fclean: clean
	@ rm -f $(NAME)
	@ printf " \033[32m \033[1mItzstd\033[22m cleaned\033[m\n"

re: fclean
	@ $(MAKE) -s -j

.PHONY: all fclean re clean

-include $(DEP)
