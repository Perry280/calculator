#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void strcat_double(char *restrict dest, double n) {
    const size_t len = (size_t)snprintf(NULL, 0, "%g", n) + 1;
    char buf[len];
    buf[len - 1] = '\0';
    snprintf(buf, len, "%g", n);
    strcat(dest, buf);
}

static size_t compute_token_string_len(const gvec_t *tokens, int include_eoi) {
    size_t string_len = 0;
    int eoi_in_tokens = 0;
    const size_t size = gvec_size(tokens);
    const token_t *const arr = (const token_t *)gvec_array(tokens);
    for (size_t i = 0; i < size; i++) {
        switch (arr[i].type) {
            case TT_INVALID:
                string_len += strlen("INVALID");
                break;
            case TT_NUMBER:
                string_len += (size_t)snprintf(NULL, 0, "%g", arr[i].word);
                break;
            case TT_EOI:
                eoi_in_tokens = 1;
                string_len++;
                break;
            default:
                string_len++;
                break;
        }
    }

    // Add enough space for white spaces between the tokens
    string_len += size - (size_t)(eoi_in_tokens * (include_eoi == 0)) * 2;

    return string_len;
}

static char *create_token_string(const gvec_t *tokens,
                                 int include_eoi,
                                 size_t string_len) {
    char *const string = calloc(string_len, sizeof(char));
    if (string == NULL)
        return NULL;

    const size_t size = gvec_size(tokens);
    const token_t *const arr = (const token_t *)gvec_array(tokens);
    for (size_t i = 0; i < size; i++) {
        char *symbol = NULL;
        switch (arr[i].type) {
            case TT_INVALID:
                symbol = "INVALID";
                break;
            case TT_OPEN_PARENTHESIS:
                symbol = "(";
                break;
            case TT_CLOSE_PARENTHESIS:
                symbol = ")";
                break;
            case TT_ADDITION:
                symbol = "+";
                break;
            case TT_SUBTRACTION:
                symbol = "-";
                break;
            case TT_MULTIPLICATION:
                symbol = "*";
                break;
            case TT_DIVISION:
                symbol = "/";
                break;
            case TT_POWER:
                symbol = "^";
                break;
            case TT_NUMBER:
                strcat_double(string, arr[i].word);
                break;
            case TT_EOI:
                if (include_eoi)
                    symbol = "$";
                break;
        }

        if (symbol != NULL)
            strcat(string, symbol);

        if (i < size - 1)
            strcat(string, " ");
    }

    return string;
}

char *tokens_to_string(const gvec_t *tokens, int include_eoi) {
    if (tokens == NULL || gvec_size(tokens) == 0
        || (!include_eoi && gvec_size(tokens) == 1
            && ((token_t *)gvec_ptr_at(tokens, 0))->type == TT_EOI))
        return NULL;

    const size_t string_len = compute_token_string_len(tokens, include_eoi);
    if (string_len == 0)
        return NULL;

    return create_token_string(tokens, include_eoi, string_len);
}

void print_tokens(const gvec_t *tokens, int include_eoi) {
    char *token_string = tokens_to_string(tokens, include_eoi);
    if (token_string == NULL)
        return;

    printf("%s\n", token_string);
    free(token_string);
}

#define ERR_MSG_FIRST_LINE_START  "Input: "
#define ERR_MSG_SECOND_LINE_START "ERROR: "

static char *format_err_msg(const char *input, err_msg_t err) {
    if (input == NULL || err.msg == NULL || strlen(input) == 0
        || strlen(err.msg) == 0)
        return NULL;

    const size_t first_line_len =
        strlen(ERR_MSG_FIRST_LINE_START) + strlen(input) + strlen("\n");

    const size_t second_line_len = strlen(ERR_MSG_SECOND_LINE_START)
                                   + err.loc.index + strlen("^ ")
                                   + strlen(err.msg) + 1;

    char *formated_err = calloc(first_line_len + second_line_len, sizeof(char));
    if (formated_err == NULL)
        return NULL;

    strcat(formated_err, ERR_MSG_FIRST_LINE_START);
    strcat(formated_err, input);
    strcat(formated_err, "\n");
    strcat(formated_err, ERR_MSG_SECOND_LINE_START);

    const size_t temp_len = first_line_len + strlen(ERR_MSG_SECOND_LINE_START);
    for (size_t i = 0; i < err.loc.index; i++)
        formated_err[temp_len + i] = ' ';

    strcat(formated_err, "^ ");
    strcat(formated_err, err.msg);

    return formated_err;
}

void print_err_msg(const char *input, err_msg_t err) {
    char *const formatted_err = format_err_msg(input, err);
    if (formatted_err == NULL)
        return;

    fprintf(stderr, "%s\n", formatted_err);
    free(formatted_err);
}

#define RES_FIRST_LINE_START  "Input:  "
#define RES_SECOND_LINE_START "Result: "

static char *format_expr_res(const char *input, double expr_res) {
    if (input == NULL || strlen(input) == 0)
        return NULL;

    const size_t first_line_len =
        strlen(RES_FIRST_LINE_START) + strlen(input) + strlen("\n");

    const size_t second_line_len = strlen(RES_SECOND_LINE_START)
                                   + (size_t)snprintf(NULL, 0, "%g", expr_res)
                                   + 1;

    char *formated_res = calloc(first_line_len + second_line_len, sizeof(char));
    if (formated_res == NULL)
        return NULL;

    strcat(formated_res, RES_FIRST_LINE_START);
    strcat(formated_res, input);
    strcat(formated_res, "\n");
    strcat(formated_res, RES_SECOND_LINE_START);
    strcat_double(formated_res, expr_res);

    return formated_res;
}

void print_expr_res(const char *input, double expr_res) {
    char *formated_res = format_expr_res(input, expr_res);
    if (formated_res == NULL)
        return;

    printf("%s\n", formated_res);
    free(formated_res);
}
