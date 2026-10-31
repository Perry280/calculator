#include "parser.h"
#include "lexer.h"
#include <stddef.h>
#include <stdio.h>

enum non_terminals {
    NT_A,
    NT_B0,
    NT_B1,
    NT_C0,
    NT_C1,
    NT_D0,
    NT_D1,
    NT_X
};

typedef struct err_msg_s {
    char *msg;
    size_t index; /**< Index of the token that caused the error. */
} err_msg_t;

static int rd_A(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg);
static int rd_B0(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg);
static int rd_B1(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg);
static int rd_C0(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg);
static int rd_C1(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg);
static int rd_D0(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg);
static int rd_D1(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg);
static int rd_X(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg);

int recursive_descent_parser(const gvec_t *tokens, char **msg) {
    if (tokens == NULL || tokens->size == 0 || msg == NULL)
        return P_EINVAL;

    if (tokens->size == 1
        && ((token_t *)GVEC_PTR_AT(tokens, 0))->type == TT_EOI)
        return P_OK;

    gvec_iter_t *iter = gvec_iter_create(tokens);
    if (iter == NULL)
        return P_ENOMEM;

    err_msg_t err_msg = {0};
    token_t token = {0};
    gvec_iter_next(iter, &token);

    int res = rd_A(iter, &token, &err_msg);

    if (res == P_OK && token.type != TT_EOI) {
        err_msg.msg = "Unexpected symbol.";
        res = P_SYNERR;
    }

    if (res == P_SYNERR) {
        // TODO
    }

    gvec_iter_free(iter);
    return res;
}

static int rd_A(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg) {
    switch (token->type) {
        case TT_ADDITION:
        case TT_SUBTRACTION:
            err_msg->index++;
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                err_msg->msg = "Symbol expected.";
                return P_SYNERR;
            }

            // fallthrough
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_B0(iter, token, err_msg);
        default:
            err_msg->msg = "Operand, + or - expected.";
            return P_SYNERR;
    }
}

static int rd_B0(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_C0(iter, token, err_msg) && rd_B1(iter, token, err_msg);
        default:
            err_msg->msg = "Operand expected.";
            return P_SYNERR;
    }
}

static int rd_B1(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg) {
    switch (token->type) {
        case TT_ADDITION:
        case TT_SUBTRACTION:
            err_msg->index++;
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                err_msg->msg = "Symbol expected.";
                return P_SYNERR;
            }

            return rd_C0(iter, token, err_msg) && rd_B1(iter, token, err_msg);
        case TT_CLOSE_PARENTHESIS:
        case TT_EOI:
            return P_OK;
        default:
            err_msg->msg = "Close paren, + or - expected.";
            return P_SYNERR;
    }
}

static int rd_C0(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_D0(iter, token, err_msg) && rd_C1(iter, token, err_msg);
        default:
            err_msg->msg = "Operand expected.";
            return P_SYNERR;
    }
}

static int rd_C1(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg) {
    switch (token->type) {
        case TT_MULTIPLICATION:
        case TT_DIVISION:
            err_msg->index++;
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                err_msg->msg = "Symbol expected.";
                return P_SYNERR;
            }

            return rd_D0(iter, token, err_msg) && rd_C1(iter, token, err_msg);
        case TT_CLOSE_PARENTHESIS:
        case TT_ADDITION:
        case TT_SUBTRACTION:
        case TT_EOI:
            return P_OK;
        default:
            err_msg->msg = "Close paren, +, -, *, or / expected.";
            return P_SYNERR;
    }
}

static int rd_D0(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_X(iter, token, err_msg) && rd_D1(iter, token, err_msg);
        default:
            err_msg->msg = "Operand expected.";
            return P_SYNERR;
    }
}

static int rd_D1(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg) {
    switch (token->type) {
        case TT_POWER:
            err_msg->index++;
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                err_msg->msg = "Symbol expected.";
                return P_SYNERR;
            }

            return rd_D0(iter, token, err_msg);
        case TT_CLOSE_PARENTHESIS:
        case TT_ADDITION:
        case TT_SUBTRACTION:
        case TT_MULTIPLICATION:
        case TT_DIVISION:
        case TT_EOI:
            return P_OK;
        default:
            err_msg->msg = "Close paren or operator expected.";
            return P_SYNERR;
    }
}

static int rd_X(gvec_iter_t *iter, token_t *token, err_msg_t *err_msg) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
            err_msg->index++;
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                err_msg->msg = "Symbol expected.";
                return P_SYNERR;
            }

            if (!rd_A(iter, token, err_msg)
                && token->type != TT_CLOSE_PARENTHESIS) {
                err_msg->msg = "Close paren expected.";
                return P_SYNERR;
            }

            // fallthrough
        case TT_NUMBER:
            err_msg->index++;
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                err_msg->msg = "Symbol expected.";
                return P_SYNERR;
            }
            return P_OK;
        default:
            err_msg->msg = "Operand expected.";
            return P_SYNERR;
    }
}

// -------- LL(1) parser --------

// int LL1_parser(const gvec_t *tokens) {
//     if (tokens == NULL || tokens->size == 0
//         || (tokens->size == 1
//             && ((token_t *)GVEC_PTR_AT(tokens, 0))->type == TT_EOI))
//         return 1;

//     gvec_iter_t *iter = gvec_iter_create(tokens);
//     if (iter == NULL)
//         return 0;

//     token_t token = {0};
//     gvec_iter_next(iter, &token);
//     return 0;
// }
