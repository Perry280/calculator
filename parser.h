#ifndef PARSER_H
#define PARSER_H

#include "generic_data_structures/generic_vector/generic_vector.h"

#define P_OK     0
#define P_ENOMEM -1
#define P_SYNERR -2

/*
 * n: number
 * e: empty string
 * $: end of input
 *
 * Grammar
 * A -> + B | - B | B
 * B -> C + B | C - B | C
 * C -> D * C | D / C | D
 * D -> X ^ D | X
 * X -> ( A ) | n
 */

// -------- Top down parser --------

/*
 * Top down backtrack free (predictive) grammar
 * A -> + B0 | - B0 | B0
 * B0 -> C0 B1
 * B1 -> + C0 B1 | - C0 B1 | e
 * C0 -> D0 C1
 * C1 -> * D0 C1 | / D0 C1 | e
 * D0 -> X D1
 * D1 -> ^ D0 | e
 * X -> ( A ) | n
 */

/*
 * FIRST('+')  = { '+' }
 * FIRST('-')  = { '-' }
 * FIRST('*')  = { '*' }
 * FIRST('/')  = { '/' }
 * FIRST('^')  = { '^' }
 * FIRST('n')  = { 'n' }
 * FIRST('(')  = { '(' }
 * FIRST(')')  = { ')' }
 * FIRST('e')  = { 'e' }
 * FIRST('$')  = { '$' }
 * FIRST('A')  = { '+', '-', 'n', '(' }
 * FIRST('B0') = { 'n', '(' }
 * FIRST('B1') = { '+', '-', 'e' }
 * FIRST('C0') = { 'n', '(' }
 * FIRST('C1') = { '*', '/', 'e' }
 * FIRST('D0') = { 'n', '(' }
 * FIRST('D1') = { '^', 'e' }
 * FIRST('X')  = { 'n', '(' }
 *
 * FOLLOW('A')  = { ')', '$' }
 * FOLLOW('B0') = { ')', '$' }
 * FOLLOW('B1') = { ')', '$' }
 * FOLLOW('C0') = { '+', '-', ')', '$' }
 * FOLLOW('C1') = { '+', '-', ')', '$' }
 * FOLLOW('D0') = { '+', '-', '*', '/', ')', '$' }
 * FOLLOW('D1') = { '+', '-', '*', '/', ')', '$' }
 * FOLLOW('X')  = { '+', '-', '*', '/', '^', ')', '$' }
 *
 * AUGMENTED_FIRST('A -> B0')       = { 'n', '(' }
 * AUGMENTED_FIRST('A -> + B0')     = { '+' }
 * AUGMENTED_FIRST('A -> - B0')     = { '-' }
 * AUGMENTED_FIRST('B0 -> C0 B1')   = { 'n', '(' }
 * AUGMENTED_FIRST('B1 -> + C0 B1') = { '+' }
 * AUGMENTED_FIRST('B1 -> - C0 B1') = { '-' }
 * AUGMENTED_FIRST('B1 -> e')       = { ')', 'e', '$', }
 * AUGMENTED_FIRST('C0 -> D0 C1')   = { 'n', '(' }
 * AUGMENTED_FIRST('C1 -> * D0 C1') = { '*' }
 * AUGMENTED_FIRST('C1 -> / D0 C1') = { '/' }
 * AUGMENTED_FIRST('C1 -> e')       = { '+', '-', ')', 'e', '$' }
 * AUGMENTED_FIRST('D0 -> X D1')    = { 'n', '(' }
 * AUGMENTED_FIRST('D1 -> ^ D0')    = { '^' }
 * AUGMENTED_FIRST('D1 -> e')       = { '+', '-', '*', '/', ')', 'e', '$', }
 * AUGMENTED_FIRST('X -> ( A )')    = { '(' }
 * AUGMENTED_FIRST('X -> n')        = { 'n' }
 */

int recursive_descent_parser(const gvec_t *tokens);

/*
 * LL(1) table
 * +----+-------+---+---------+---------+---------+---------+------+-------+---+
 * |    | (     | ) | *       | +       | -       | /       | ^    | n     | $ |
 * +----+-------+---+---------+---------+---------+---------+------+-------+---+
 * | A  | B0    |   |         | + B0    | - B0    |         |      | B0    |   |
 * | B0 | C0 B1 |   |         |         |         |         |      | C0 B1 |   |
 * | B1 |       | e |         | + C0 B1 | - C0 B1 |         |      |       | e |
 * | C0 | D0 C1 |   |         |         |         |         |      | D0 C1 |   |
 * | C1 |       | e | * D0 C1 | e       | e       | / D0 C1 |      |       | e |
 * | D0 | X D1  |   |         |         |         |         |      | X D1  |   |
 * | D1 |       | e | e       | e       | e       | e       | ^ D0 |       | e |
 * | X  | ( A ) |   |         |         |         |         |      | n     |   |
 * +----+-------+---+---------+---------+---------+---------+------+-------+---+
 */

// int LL1_parser(const gvec_t *tokens);

#endif
