#include "parser.h"

#define SYMBOL_ERR     "Symbol expected."
#define OPERAND_ERR    "Operand expected."
#define UNEXPECTED_ERR "Unexpected symbol."
#define A_ERR          "Operand, + or - expected."
#define B1_ERR         "Close paren, + or - expected."
#define C1_ERR         "Close paren, +, -, *, or / expected."
#define D1_ERR         "Close paren or operator expected."
#define X_ERR          "Close paren expected."

typedef struct parsing_result_s {
    err_msg_t err;
    int ret_code;
} parsing_result_t;

static parsing_result_t rd_A(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_B0(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_B1(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_C0(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_C1(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_D0(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_D1(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_X(gvec_iter_t *iter, token_t *token);

static inline parsing_result_t set_syn_err(const char *err_msg,
                                           location_t location) {
    const parsing_result_t res = {.ret_code = P_SYNERR,
                                  .err = {.msg = err_msg, .loc = location}};
    return res;
}

static inline parsing_result_t set_syn_err_eoi(location_t last_location) {
    const location_t eoi = {.index = last_location.index + last_location.length,
                            .length = 1};
    return set_syn_err(SYMBOL_ERR, eoi);
}

static inline parsing_result_t set_success(void) {
    const parsing_result_t res = {.ret_code = P_OK, .err = {0}};
    return res;
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

    parsing_result_t res = rd_A(iter, &token);
    if (res.ret_code == P_OK && token.type != TT_EOI)
        res = set_syn_err(UNEXPECTED_ERR, token.loc);

    if (res.ret_code != P_OK)
        *err = res.err;

    gvec_iter_free(iter);
    return res.ret_code;
}

static parsing_result_t rd_A(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_ADDITION:
        case TT_SUBTRACTION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);
            // fallthrough
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_B0(iter, token);
        default:
            return set_syn_err(A_ERR, token->loc);
    }
}

static parsing_result_t rd_B0(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            const parsing_result_t res = rd_C0(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            return rd_B1(iter, token);
        default:
            return set_syn_err(OPERAND_ERR, token->loc);
    }
}

static parsing_result_t rd_B1(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_ADDITION:
        case TT_SUBTRACTION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            const parsing_result_t res = rd_C0(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            return rd_B1(iter, token);
        case TT_CLOSE_PARENTHESIS:
        case TT_EOI:
            return set_success();
        default:
            return set_syn_err(B1_ERR, token->loc);
    }
}

static parsing_result_t rd_C0(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            const parsing_result_t res = rd_D0(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            return rd_C1(iter, token);
        default:
            return set_syn_err(OPERAND_ERR, token->loc);
    }
}

static parsing_result_t rd_C1(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_MULTIPLICATION:
        case TT_DIVISION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            const parsing_result_t res = rd_D0(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            return rd_C1(iter, token);
        case TT_CLOSE_PARENTHESIS:
        case TT_ADDITION:
        case TT_SUBTRACTION:
        case TT_EOI:
            return set_success();
        default:
            return set_syn_err(C1_ERR, token->loc);
    }
}

static parsing_result_t rd_D0(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            const parsing_result_t res = rd_X(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            return rd_D1(iter, token);
        default:
            return set_syn_err(OPERAND_ERR, token->loc);
    }
}

static parsing_result_t rd_D1(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_POWER:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            return rd_D0(iter, token);
        case TT_CLOSE_PARENTHESIS:
        case TT_ADDITION:
        case TT_SUBTRACTION:
        case TT_MULTIPLICATION:
        case TT_DIVISION:
        case TT_EOI:
            return set_success();
        default:
            return set_syn_err(D1_ERR, token->loc);
    }
}

static parsing_result_t rd_X(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            const parsing_result_t res = rd_A(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            if (token->type != TT_CLOSE_PARENTHESIS)
                return set_syn_err(X_ERR, token->loc);

            // fallthrough
        case TT_NUMBER:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            return set_success();
        default:
            return set_syn_err(OPERAND_ERR, token->loc);
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
