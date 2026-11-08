#include "parser.h"
#include "types.h"
#include <math.h>

#define SYMBOL_ERR     "Symbol expected."
#define OPERAND_ERR    "Operand expected."
#define UNEXPECTED_ERR "Unexpected symbol."
#define A0_ERR         "Operand, + or - expected."
#define A1_ERR         "Close paren, + or - expected."
#define B1_ERR         "Close paren, +, -, *, or / expected."
#define C1_ERR         "Close paren or operator expected."
#define S_ERR          A0_ERR
#define X_ERR          "Close paren expected."

typedef struct parsing_result_s {
    err_msg_t err;
    double expr_res;
    int ret_code;
} parsing_result_t;

static parsing_result_t rd_A0(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_A1(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_B0(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_B1(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_C0(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_C1(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_S(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_X(gvec_iter_t *iter, token_t *token);

static inline parsing_result_t set_syn_err(const char *err_msg,
                                           location_t location) {
    const parsing_result_t res = {.ret_code = P_SYNERR,
                                  .expr_res = 0,
                                  .err = {.msg = err_msg, .loc = location}};
    return res;
}

static inline parsing_result_t set_syn_err_eoi(location_t last_location) {
    const location_t eoi = {.index = last_location.index + last_location.length,
                            .length = 1};
    return set_syn_err(SYMBOL_ERR, eoi);
}

static inline parsing_result_t set_success(double num) {
    const parsing_result_t res = {.ret_code = P_OK,
                                  .expr_res = num,
                                  .err = {0}};
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

    parsing_result_t res = rd_A0(iter, &token);
    if (res.ret_code == P_OK && token.type != TT_EOI)
        res = set_syn_err(UNEXPECTED_ERR, token.loc);

    if (res.ret_code != P_OK)
        *err = res.err;

    gvec_iter_free(iter);
    return res.ret_code;
}

static parsing_result_t rd_A0(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_ADDITION:
        case TT_SUBTRACTION:
        case TT_NUMBER:
            const parsing_result_t res1 = rd_S(iter, token);
            if (res1.ret_code == P_SYNERR)
                return res1;

            const parsing_result_t res2 = rd_B0(iter, token);
            if (res2.ret_code == P_SYNERR)
                return res2;

            return rd_A1(iter, token);
        default:
            return set_syn_err(A0_ERR, token->loc);
    }
}

static parsing_result_t rd_A1(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_ADDITION:
        case TT_SUBTRACTION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            const parsing_result_t res = rd_B0(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            return rd_A1(iter, token);
        case TT_EOI:
        case TT_CLOSE_PARENTHESIS:
            return set_success(0);
        default:
            return set_syn_err(A1_ERR, token->loc);
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
        case TT_MULTIPLICATION:
        case TT_DIVISION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            const parsing_result_t res = rd_C0(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            return rd_B1(iter, token);
        case TT_EOI:
        case TT_CLOSE_PARENTHESIS:
        case TT_ADDITION:
        case TT_SUBTRACTION:
            return set_success(0);
        default:
            return set_syn_err(B1_ERR, token->loc);
    }
}

static parsing_result_t rd_C0(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            parsing_result_t res = rd_X(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            const parsing_result_t res_tmp = rd_C1(iter, token);
            if (res_tmp.ret_code == P_SYNERR)
                return res_tmp;

            res.expr_res = pow(res.expr_res, res_tmp.expr_res);
            return res;
        default:
            return set_syn_err(OPERAND_ERR, token->loc);
    }
}

static parsing_result_t rd_C1(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_POWER:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            return rd_C0(iter, token);
        case TT_EOI:
        case TT_CLOSE_PARENTHESIS:
        case TT_MULTIPLICATION:
        case TT_ADDITION:
        case TT_SUBTRACTION:
        case TT_DIVISION:
            return set_success(1);
        default:
            return set_syn_err(C1_ERR, token->loc);
    }
}

static parsing_result_t rd_S(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_ADDITION:
        case TT_SUBTRACTION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);
            // fallthrough
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return set_success(0);
        default:
            return set_syn_err(S_ERR, token->loc);
    }
}

static parsing_result_t rd_X(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            const parsing_result_t res = rd_A0(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            if (token->type != TT_CLOSE_PARENTHESIS)
                return set_syn_err(X_ERR, token->loc);

            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            return res;
        case TT_NUMBER:
            const double word = token->word;
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            return set_success(word);
        default:
            return set_syn_err(OPERAND_ERR, token->loc);
    }
}
