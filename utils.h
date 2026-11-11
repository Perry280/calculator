#ifndef UTILS_H
#define UTILS_H

#include "generic_vector.h"
#include "types.h"

char *tokens_to_string(const gvec_t *tokens, int include_eoi);
void print_tokens(const gvec_t *tokens, int include_eoi);
void print_err_msg(const char *input, err_msg_t err);
void print_expr_res(const char *input, double expr_res);

#endif
