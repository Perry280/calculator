#include "parser.h"
#include "types.h"
#include <math.h>

#define SYMBOL_ERR     "Symbol expected."
#define OPERAND_ERR    "Operand expected."
#define UNEXPECTED_ERR "Unexpected symbol."
#define EXPR_ERR       "Operand, + or - expected."
#define ADDTAIL_ERR    "Close paren, + or - expected."
#define MULTAIL_ERR    "Close paren, +, -, *, or / expected."
#define POWTAIL_ERR    "Close paren or operator expected."
#define TERM_ERR       "Close paren expected."

typedef struct parsing_result_s {
    err_msg_t err;
    double expr_res;
    int ret_code;
} parsing_result_t;

static parsing_result_t rd_Expr(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_AddTail(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_Mul(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_MulTail(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_Power(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_PowTail(gvec_iter_t *iter, token_t *token);
static parsing_result_t rd_Term(gvec_iter_t *iter, token_t *token);

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

    parsing_result_t res = rd_Expr(iter, &token);
    if (res.ret_code == P_OK && token.type != TT_EOI)
        res = set_syn_err(UNEXPECTED_ERR, token.loc);

    if (res.ret_code != P_OK)
        *err = res.err;

    gvec_iter_free(iter);
    return res.ret_code;
}

static parsing_result_t rd_Expr(gvec_iter_t *iter, token_t *token) {
    int is_unary_minus = 0;
    switch (token->type) {
        case TT_SUBTRACTION:
            is_unary_minus = 1;
            // fallthrough
        case TT_ADDITION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);
            // fallthrough
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            parsing_result_t mul_res = rd_Mul(iter, token);
            if (mul_res.ret_code == P_SYNERR)
                return mul_res;

            if (is_unary_minus)
                mul_res.expr_res = -mul_res.expr_res;

            return rd_AddTail(iter, token);
        default:
            return set_syn_err(EXPR_ERR, token->loc);
    }
}

static parsing_result_t rd_AddTail(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_ADDITION:
        case TT_SUBTRACTION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            const parsing_result_t res = rd_Mul(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            return rd_AddTail(iter, token);
        case TT_EOI:
        case TT_CLOSE_PARENTHESIS:
            return set_success(0);
        default:
            return set_syn_err(ADDTAIL_ERR, token->loc);
    }
}

static parsing_result_t rd_Mul(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            const parsing_result_t res = rd_Power(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            return rd_MulTail(iter, token);
        default:
            return set_syn_err(OPERAND_ERR, token->loc);
    }
}

static parsing_result_t rd_MulTail(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_MULTIPLICATION:
        case TT_DIVISION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            const parsing_result_t res = rd_Power(iter, token);
            if (res.ret_code == P_SYNERR)
                return res;

            return rd_MulTail(iter, token);
        case TT_EOI:
        case TT_CLOSE_PARENTHESIS:
        case TT_ADDITION:
        case TT_SUBTRACTION:
            return set_success(0);
        default:
            return set_syn_err(MULTAIL_ERR, token->loc);
    }
}

static parsing_result_t rd_Power(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            parsing_result_t term_res = rd_Term(iter, token);
            if (term_res.ret_code == P_SYNERR)
                return term_res;

            const parsing_result_t powtail_res = rd_PowTail(iter, token);
            if (powtail_res.ret_code == P_SYNERR)
                return powtail_res;

            term_res.expr_res = pow(term_res.expr_res, powtail_res.expr_res);
            return term_res;
        default:
            return set_syn_err(OPERAND_ERR, token->loc);
    }
}

static parsing_result_t rd_PowTail(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_POWER:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            return rd_Power(iter, token);
        case TT_EOI:
        case TT_CLOSE_PARENTHESIS:
        case TT_MULTIPLICATION:
        case TT_ADDITION:
        case TT_SUBTRACTION:
        case TT_DIVISION:
            return set_success(1);
        default:
            return set_syn_err(POWTAIL_ERR, token->loc);
    }
}

static parsing_result_t rd_Term(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            const parsing_result_t expr_res = rd_Expr(iter, token);
            if (expr_res.ret_code == P_SYNERR)
                return expr_res;

            if (token->type != TT_CLOSE_PARENTHESIS)
                return set_syn_err(TERM_ERR, token->loc);

            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            return expr_res;
        case TT_NUMBER:
            const double word = token->word;
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return set_syn_err_eoi(token->loc);

            return set_success(word);
        default:
            return set_syn_err(OPERAND_ERR, token->loc);
    }
}
