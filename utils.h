#ifndef UTILS_H
#define UTILS_H

#include "generic_vector.h"
#include "types.h"

char *tokens_to_string(const gvec_t *tokens, int include_eoi);
char *format_err_msg(const char *input, err_msg_t err);
void print_err_msg(const char *input, err_msg_t err);

#endif
