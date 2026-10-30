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

static int rd_A(gvec_iter_t *iter, token_t *token);
static int rd_B0(gvec_iter_t *iter, token_t *token);
static int rd_B1(gvec_iter_t *iter, token_t *token);
static int rd_C0(gvec_iter_t *iter, token_t *token);
static int rd_C1(gvec_iter_t *iter, token_t *token);
static int rd_D0(gvec_iter_t *iter, token_t *token);
static int rd_D1(gvec_iter_t *iter, token_t *token);
static int rd_X(gvec_iter_t *iter, token_t *token);

int recursive_descent_parser(const gvec_t *tokens) {
    if (tokens == NULL || tokens->size == 0
        || (tokens->size == 1
            && ((token_t *)GVEC_PTR_AT(tokens, 0))->type == TT_EOI))
        return P_OK;

    gvec_iter_t *iter = gvec_iter_create(tokens);
    if (iter == NULL)
        return P_ENOMEM;

    token_t token = {0};
    gvec_iter_next(iter, &token);

    int res = rd_A(iter, &token);

    if (res == P_OK && token.type != TT_EOI)
        res = P_SYNERR;

    gvec_iter_free(iter);
    return res;
}

static int rd_A(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_ADDITION:
        case TT_SUBTRACTION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return P_SYNERR;

            // fallthrough
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_B0(iter, token);
        default:
            return P_SYNERR;
    }
}

static int rd_B0(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_C0(iter, token) && rd_B1(iter, token);
        default:
            return P_SYNERR;
    }
}

static int rd_B1(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_ADDITION:
        case TT_SUBTRACTION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return P_SYNERR;

            return rd_C0(iter, token) && rd_B1(iter, token);
        case TT_CLOSED_PARENTHESIS:
        case TT_EOI:
            return P_OK;
        default:
            return P_SYNERR;
    }
}

static int rd_C0(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_D0(iter, token) && rd_C1(iter, token);
        default:
            return P_SYNERR;
    }
}

static int rd_C1(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_MULTIPLICATION:
        case TT_DIVISION:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return P_SYNERR;

            return rd_D0(iter, token) && rd_C1(iter, token);
        case TT_CLOSED_PARENTHESIS:
        case TT_ADDITION:
        case TT_SUBTRACTION:
        case TT_EOI:
            return P_OK;
        default:
            return P_SYNERR;
    }
}

static int rd_D0(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
        case TT_NUMBER:
            return rd_X(iter, token) && rd_D1(iter, token);
        default:
            return P_SYNERR;
    }
}

static int rd_D1(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_POWER:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return P_SYNERR;

            return rd_D0(iter, token);
        case TT_CLOSED_PARENTHESIS:
        case TT_ADDITION:
        case TT_SUBTRACTION:
        case TT_MULTIPLICATION:
        case TT_DIVISION:
        case TT_EOI:
            return P_OK;
        default:
            return P_SYNERR;
    }
}

static int rd_X(gvec_iter_t *iter, token_t *token) {
    switch (token->type) {
        case TT_OPEN_PARENTHESIS:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return P_SYNERR;

            if (!rd_A(iter, token) && token->type != TT_CLOSED_PARENTHESIS)
                return P_SYNERR;

            // fallthrough
        case TT_NUMBER:
            if (gvec_iter_next(iter, token) == GVEC_EITER)
                return P_SYNERR;

            return P_OK;
        default:
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
