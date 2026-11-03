#include "parser.h"

#define SYMBOL_ERR     "Symbol expected."
#define OPERAND_ERR    "Operand expected."
#define UNEXPECTED_ERR "Unexpected symbol."
#define A_ERR          "Operand, + or - expected."
#define B1_ERR         "Close paren, + or - expected."
#define C1_ERR         "Close paren, +, -, *, or / expected."
#define D1_ERR         "Close paren or operator expected."
#define X_ERR          "Close paren expected."

static int rd_A(gvec_iter_t *iter, token_t *token, err_msg_t *err);
static int rd_B0(gvec_iter_t *iter, token_t *token, err_msg_t *err);
static int rd_B1(gvec_iter_t *iter, token_t *token, err_msg_t *err);
static int rd_C0(gvec_iter_t *iter, token_t *token, err_msg_t *err);
static int rd_C1(gvec_iter_t *iter, token_t *token, err_msg_t *err);
static int rd_D0(gvec_iter_t *iter, token_t *token, err_msg_t *err);
static int rd_D1(gvec_iter_t *iter, token_t *token, err_msg_t *err);
static int rd_X(gvec_iter_t *iter, token_t *token, err_msg_t *err);

static inline void set_err(err_msg_t *err,
                           const char *err_msg,
                           location_t location) {
    err->msg = err_msg;
    err->loc = location;
}

static inline void set_err_eoi(err_msg_t *err, location_t last_location) {
    const location_t eoi = {.index = last_location.index + last_location.length,
                            .length = 1};
    set_err(err, SYMBOL_ERR, eoi);
}

int recursive_descent_parser(const gvec_t *tokens, err_msg_t *err) {
    if (tokens == NULL || gvec_size(tokens) == 0 || err == NULL)
        return P_EINVAL;

    if (gvec_size(tokens) == 1
        && ((token_t *)gvec_ptr_at(tokens, 0))->type == TT_EOI)
        return P_OK;

    gvec_iter_t *iter = gvec_iter_create(tokens);
    if (iter == NULL)
        return P_ENOMEM;

    token_t token = {0};
    gvec_iter_next(iter, &token);

    int res = rd_A(iter, &token, err);
    if (res == P_OK && token.type != TT_EOI) {
        set_err(err, UNEXPECTED_ERR, token.loc);
        res = P_SYNERR;
    }

    gvec_iter_free(iter);
    return res;
}

static int rd_A(gvec_iter_t *iter, token_t *token, err_msg_t *err) {
    switch (token->type) {
        case TT_ADDITION:
        case TT_SUBTRACTION:
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                set_err_eoi(err, token->loc);
                return P_SYNERR;
            }

            // fallthrough
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_B0(iter, token, err);
        default:
            set_err(err, A_ERR, token->loc);
            return P_SYNERR;
    }
}

static int rd_B0(gvec_iter_t *iter, token_t *token, err_msg_t *err) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_C0(iter, token, err) && rd_B1(iter, token, err);
        default:
            set_err(err, OPERAND_ERR, token->loc);
            return P_SYNERR;
    }
}

static int rd_B1(gvec_iter_t *iter, token_t *token, err_msg_t *err) {
    switch (token->type) {
        case TT_ADDITION:
        case TT_SUBTRACTION:
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                set_err_eoi(err, token->loc);
                return P_SYNERR;
            }

            return rd_C0(iter, token, err) && rd_B1(iter, token, err);
        case TT_CLOSE_PARENTHESIS:
        case TT_EOI:
            return P_OK;
        default:
            set_err(err, B1_ERR, token->loc);
            return P_SYNERR;
    }
}

static int rd_C0(gvec_iter_t *iter, token_t *token, err_msg_t *err) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_D0(iter, token, err) && rd_C1(iter, token, err);
        default:
            set_err(err, OPERAND_ERR, token->loc);
            return P_SYNERR;
    }
}

static int rd_C1(gvec_iter_t *iter, token_t *token, err_msg_t *err) {
    switch (token->type) {
        case TT_MULTIPLICATION:
        case TT_DIVISION:
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                set_err_eoi(err, token->loc);
                return P_SYNERR;
            }

            return rd_D0(iter, token, err) && rd_C1(iter, token, err);
        case TT_CLOSE_PARENTHESIS:
        case TT_ADDITION:
        case TT_SUBTRACTION:
        case TT_EOI:
            return P_OK;
        default:
            set_err(err, C1_ERR, token->loc);
            return P_SYNERR;
    }
}

static int rd_D0(gvec_iter_t *iter, token_t *token, err_msg_t *err) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_X(iter, token, err) && rd_D1(iter, token, err);
        default:
            set_err(err, OPERAND_ERR, token->loc);
            return P_SYNERR;
    }
}

static int rd_D1(gvec_iter_t *iter, token_t *token, err_msg_t *err) {
    switch (token->type) {
        case TT_POWER:
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                set_err_eoi(err, token->loc);
                return P_SYNERR;
            }

            return rd_D0(iter, token, err);
        case TT_CLOSE_PARENTHESIS:
        case TT_ADDITION:
        case TT_SUBTRACTION:
        case TT_MULTIPLICATION:
        case TT_DIVISION:
        case TT_EOI:
            return P_OK;
        default:
            set_err(err, D1_ERR, token->loc);
            return P_SYNERR;
    }
}

static int rd_X(gvec_iter_t *iter, token_t *token, err_msg_t *err) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                set_err_eoi(err, token->loc);
                return P_SYNERR;
            }

            if (!rd_A(iter, token, err)
                && token->type != TT_CLOSE_PARENTHESIS) {
                set_err(err, X_ERR, token->loc);
                return P_SYNERR;
            }

            // fallthrough
        case TT_NUMBER:
            if (gvec_iter_next(iter, token) == GVEC_EITER) {
                set_err_eoi(err, token->loc);
                return P_SYNERR;
            }
            return P_OK;
        default:
            set_err(err, OPERAND_ERR, token->loc);
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
