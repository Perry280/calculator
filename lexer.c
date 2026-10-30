#include "lexer.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define INVALID_CHAR(c)                                                        \
    (c) != '(' && (c) != ')' && (c) != '+' && (c) != '-' && (c) != '*'         \
        && (c) != '^' && (c) != '/' && (c) != '.' && (c) != ' '                \
        && ((c) < '0' || (c) > '9')

#define CASE_DIGITS                                                            \
    case '0':                                                                  \
    case '1':                                                                  \
    case '2':                                                                  \
    case '3':                                                                  \
    case '4':                                                                  \
    case '5':                                                                  \
    case '6':                                                                  \
    case '7':                                                                  \
    case '8':                                                                  \
    case '9'

int token_type_cmp(const void *a, const void *b) {
    return ((const token_t *)a)->type != ((const token_t *)b)->type;
}

static int push_number(gvec_t *tokens, const char *input, size_t offset) {
    token_t token = {.type = TT_NUMBER, .word = strtod(input + offset, NULL)};
    return gvec_push_back(tokens, &token);
}

static token_type_t symbol_accept(char c) {
    switch (c) {
        case '(':
            return TT_OPEN_PARENTHESIS;
        case ')':
            return TT_CLOSED_PARENTHESIS;
        case '+':
            return TT_ADDITION;
        case '-':
            return TT_SUBTRACTION;
        case '*':
            return TT_MULTIPLICATION;
        case '/':
            return TT_DIVISION;
        case '^':
            return TT_POWER;
        case ' ':
            return TT_INVALID;
    }

    return TT_INVALID;
}

int tokenize(const char *input, gvec_t **tokens) {
    if (input == NULL || strlen(input) == 0)
        return L_EINVAL;

    gvec_t *t = gvec_create(sizeof(token_t), token_type_cmp, NULL);
    if (t == NULL)
        return L_ENOMEM;

    enum input_state {
        I_SYMBOLS,
        I_POINT,
        I_NUMBER,
        I_AFTER_POINT
    } state = I_SYMBOLS;

    size_t input_len = strlen(input);
    int status = L_OK;
    size_t offset = 0;
    for (size_t i = 0; i < input_len; i++) {
        const char c = input[i];
        token_t token = {0};

        if (INVALID_CHAR(c)) {
            status = L_ERROR;
            break;
        }

        switch (state) {
            case I_SYMBOLS:
                switch (c) {
                    case '.':
                        state = I_POINT;
                        offset = i;
                        break;
                    CASE_DIGITS:
                        state = I_NUMBER;
                        offset = i;
                        break;
                    default:
                        token.type = symbol_accept(c);
                        break;
                }
                break;
            case I_POINT:
                switch (c) {
                    // clang-format off
                    CASE_DIGITS:
                        state = I_AFTER_POINT;
                        break;
                    // clang-format on
                    default:
                        status = L_ERROR;
                        break;
                }
                break;
            case I_NUMBER:
            case I_AFTER_POINT:
                switch (c) {
                    case '.':
                        if (state == I_NUMBER)
                            state = I_AFTER_POINT;
                        else
                            status = L_ERROR;
                        break;
                    CASE_DIGITS:
                        break;
                    default:
                        if (push_number(t, input, offset) == GVEC_ENOMEM) {
                            status = L_ENOMEM;
                            break;
                        }

                        state = I_SYMBOLS;
                        token.type = symbol_accept(c);
                        break;
                }
                break;
        }

        if (status != L_OK)
            break;

        if (token.type != TT_INVALID
            && gvec_push_back(t, &token) == GVEC_ENOMEM) {
            status = L_ENOMEM;
            break;
        }
    }

    if (state == I_POINT)
        status = L_ERROR;

    if (status == L_OK) {
        if ((state == I_NUMBER || state == I_AFTER_POINT)
            && push_number(t, input, offset) == GVEC_ENOMEM)
            status = L_ENOMEM;

        token_t token = {.type = TT_EOI, .word = 0};
        if (status != L_ENOMEM && gvec_push_back(t, &token) == GVEC_ENOMEM)
            status = L_ENOMEM;

        if (status == L_ENOMEM)
            gvec_free(t);
        else
            *tokens = t;
    } else
        gvec_free(t);

    return status;
}
