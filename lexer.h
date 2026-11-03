#ifndef LEXER_H
#define LEXER_H

#include "generic_data_structures/generic_vector/generic_vector.h"
#include "types.h"

#define L_OK     0
#define L_ENOMEM -1
#define L_EINVAL -2
#define L_ERROR  -3

int tokenize(const char *input, gvec_t **tokens, err_msg_t *err);

#endif
