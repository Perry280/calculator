#ifndef TIPES_H
#define TIPES_H

#include <stddef.h>

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

typedef struct location_s {
    size_t index;
    size_t length;
} location_t;

typedef struct token_s {
    location_t loc;
    double word;
    token_type_t type;
} token_t;

typedef struct err_msg_s {
    const char *msg;
    location_t loc;
} err_msg_t;

#endif
