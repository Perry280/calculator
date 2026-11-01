CC       = gcc
CFLAGS   = -g -fsanitize=address,undefined -Wall -Wextra -Wpedantic -Werror
DEPFLAGS = -MMD -MP

SOFLAGS  = -O2 -DNDEBUG -g -flto -fPIC -Wall -Wextra -Wpedantic

OBJS_DIR = .objs
DEPS_DIR = .deps
SO_DIR   = .shared_objs
LIB_DIR  = ./generic_data_structures/generic_vector

# INCLUDE = -I$(LIB_DIR)

BINS = main
SRCS = lexer.c parser.c utils.c main.c
OBJS = $(addprefix $(OBJS_DIR)/, $(SRCS:.c=.o))
DEPS = $(addprefix $(DEPS_DIR)/, $(SRCS:.c=.d))

LIB_NAME = gvec
SO_SRCS  = generic_vector.c
SO_FILE  = $(SO_DIR)/lib$(LIB_NAME).so
DEPS    += $(DEPS_DIR)/$(SO_SRCS:.c=.d)

LDFLAGS = -L$(SO_DIR) -Wl,-rpath,'$$ORIGIN/$(SO_DIR)'
LDLIBS  = -l$(LIB_NAME)

.PHONY: all clean cleanall

all: $(BINS)

$(BINS): $(OBJS) $(SO_FILE)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

$(OBJS_DIR)/%.o: %.c | $(OBJS_DIR) $(DEPS_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -MF $(DEPS_DIR)/$*.d -c $< -o $@

$(SO_FILE): $(LIB_DIR)/$(SO_SRCS) | $(SO_DIR) $(DEPS_DIR)
	$(CC) $(SOFLAGS) $(DEPFLAGS) -MF $(DEPS_DIR)/$(SO_SRCS:.c=.d) -MT $@ -shared $< -o $@

$(OBJS_DIR) $(DEPS_DIR) $(SO_DIR):
	mkdir -p $@

clean:
	rm -rf $(BINS) $(OBJS_DIR) $(DEPS_DIR)

cleanall: clean
	rm -rf $(SO_DIR)

-include $(DEPS)
