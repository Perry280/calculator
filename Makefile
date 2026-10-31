CC = gcc
CFLAGS = -g -fsanitize=address,undefined -Wall -Wextra -Wpedantic -Werror
DEPFLAGS = -MMD -MP

BINS = main
SRCS = lexer.c parser.c utils.c main.c generic_data_structures/generic_vector/generic_vector.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean

all: $(BINS)

$(BINS): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	rm -f $(BINS) $(OBJS) $(DEPS)

-include $(DEPS)
