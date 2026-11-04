CC       := gcc
CFLAGS   := -g -fsanitize=address,undefined -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion
DEPFLAGS := -MMD -MP

OBJS_DIR := .objs
DEPS_DIR := .deps

LIBS     := generic_vector.c
LIBS_DIR := ./lib/generic_data_structures/generic_vector

BINS := calculator
SRCS := lexer.c parser.c utils.c $(BINS).c $(LIBS)
OBJS := $(addprefix $(OBJS_DIR)/, $(SRCS:.c=.o))
DEPS := $(addprefix $(DEPS_DIR)/, $(SRCS:.c=.d))

vpath %.c $(LIBS_DIR)

.PHONY: all clean

all: $(BINS)

$(BINS): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJS_DIR)/%.o: %.c | $(OBJS_DIR) $(DEPS_DIR)
	$(CC) $(CFLAGS) -I$(LIBS_DIR) $(DEPFLAGS) -MF $(DEPS_DIR)/$*.d -c $< -o $@

$(OBJS_DIR) $(DEPS_DIR):
	mkdir -p $@

clean:
	rm -rf $(BINS) $(OBJS_DIR) $(DEPS_DIR)

-include $(DEPS)
