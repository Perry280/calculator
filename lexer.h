#ifndef LEXER_H
#define LEXER_H

#include "generic_data_structures/generic_vector/generic_vector.h"

#define L_OK     0
#define L_ENOMEM -1
#define L_EINVAL -2
#define L_ERROR  -3

typedef enum token_type_e {
    TT_INVALID,
    TT_OPEN_PARENTHESIS,
    TT_CLOSE_PARENTHESIS,
    TT_ADDITION,
    TT_SUBTRACTION,
    TT_MULTIPLICATION,
    TT_DIVISION,
    TT_POWER,
    TT_NUMBER,
    TT_EOI
} token_type_t;

typedef struct token_s {
    double word;
    token_type_t type;
} token_t;

int tokenize(const char *input, gvec_t **tokens);

#endif
