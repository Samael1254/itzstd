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
	@ echo " \033[33m  Compiling itzstd...\033[m"
	@ $(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LDFLAGS)
	@ tput cuu1 && tput el
	@ echo " \033[32m \033[1mItzstd\033[22m compiled\033[m"

$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.c
	@ echo " \033[33m  Compiling $(notdir $@)...\033[m"
	@ mkdir -p $(BUILD_DIR)
	@ $(CC) $(CFLAGS) -c $< -o $@
	@ tput cuu1 && tput el

all: $(NAME)

clean:
	@ rm -f $(OBJS) $(DEP)
	@ rm -rfd $(BUILD_DIR)
	@ echo " \033[32m Object files cleaned\033[m"

fclean: clean
	@ rm -f $(NAME)
	@ echo " \033[32m \033[1mItzstd\033[22m cleaned\033[m"

re: fclean
	@ $(MAKE) -s -j

.PHONY: all fclean re clean

-include $(DEP)
