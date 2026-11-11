#ifndef PARSER_H
#define PARSER_H

#include "generic_vector.h"
#include "types.h"

#define P_OK     0
#define P_ENOMEM -1
#define P_EINVAL -2
#define P_SYNERR -3

/*
 * n: number
 * e: empty string
 * $: end of input
 */

// -------- Top down parser --------

/*
 * Top down backtrack free (predictive) grammar
 * Expr    -> + Mul AddTail | - Mul AddTail | Mul AddTail
 * AddTail -> + Mul AddTail | - Mul AddTail | e
 * Mul     -> Power MulTail
 * MulTail -> * Power MulTail | / Power MulTail | e
 * Power   -> Term PowTail
 * PowTail -> ^ Power | e
 * Term    -> n | ( Expr )
 */

/*
 * FIRST
 * '$' : { '$' }
 * '(' : { '(' }
 * ')' : { ')' }
 * '*' : { '*' }
 * '+' : { '+' }
 * '-' : { '-' }
 * '/' : { '/' }
 * '^' : { '^' }
 * 'e' : { 'e' }
 * 'n' : { 'n' }
 * 'Expr'    : { '(', '+', '-', 'n' }
 * 'AddTail' : { '+', '-', 'e' }
 * 'Mul'     : { '(', 'n' }
 * 'MulTail' : { '*', '/', 'e' }
 * 'Power'   : { '(', 'n' }
 * 'PowTail' : { '^', 'e' }
 * 'Term'    : { '(', 'n' }
 *
 * FOLLOW
 * 'Expr'    : { '$', ')' }
 * 'AddTail' : { '$', ')' }
 * 'Mul'     : { '$', ')', '+', '-' }
 * 'MulTail' : { '$', ')', '+', '-' }
 * 'Power'   : { '$', ')', '*', '+', '-', '/' }
 * 'PowTail' : { '$', ')', '*', '+', '-', '/' }
 * 'Term'    : { '$', ')', '*', '+', '-', '/', '^' }
 *
 * AUGMENTED FIRST
 * 'Expr -> + Mul AddTail'      : { '+' }
 * 'Expr -> - Mul AddTail'      : { '-' }
 * 'Expr -> Mul AddTail'        : { '(', 'n' }
 * 'AddTail -> + Mul AddTail'   : { '+' }
 * 'AddTail -> - Mul AddTail'   : { '-' }
 * 'AddTail -> e'               : { '$', ')', 'e' }
 * 'Mul -> Power MulTail'       : { '(', 'n' }
 * 'MulTail -> * Power MulTail' : { '*' }
 * 'MulTail -> / Power MulTail' : { '/' }
 * 'MulTail -> e'               : { '$', ')', '+', '-', 'e' }
 * 'Power -> Term PowTail'      : { '(', 'n' }
 * 'PowTail -> ^ Power'         : { '^' }
 * 'PowTail -> e'               : { '$', ')', '*', '+', '-', '/', 'e' }
 * 'Term -> ( Expr )'           : { '(' }
 * 'Term -> n'                  : { 'n' }
 */

//
int recursive_descent_parser(const gvec_t *tokens,
                             double *expr_res,
                             err_msg_t *err);

/* clang-format off
 *
 * LL(1) table
 * +---------+---------------+---+-----------------+---------------+---------------+-----------------+---------+---------------+---+
 * |         | (             | ) | *               | +             | -             | /               | ^       | n             | $ |
 * +---------+---------------+---+-----------------+---------------+---------------+-----------------+---------+---------------+---+
 * | Expr    | Mul AddTail   |   |                 | + Mul AddTail | - Mul AddTail |                 |         | Mul AddTail   |   |
 * | AddTail |               | e |                 | + Mul AddTail | - Mul AddTail |                 |         |               | e |
 * | Mul     | Power MulTail |   |                 |               |               |                 |         | Power MulTail |   |
 * | MulTail |               | e | * Power MulTail | e             | e             | / Power MulTail |         |               | e |
 * | Power   | Term PowTail  |   |                 |               |               |                 |         | Term PowTail  |   |
 * | PowTail |               | e | e               | e             | e             | e               | ^ Power |               | e |
 * | Term    | ( Expr )      |   |                 |               |               |                 |         | n             |   |
 * +---------+---------------+---+-----------------+---------------+---------------+-----------------+---------+---------------+---+
 * 
 * clang-format on
 */

#endif
