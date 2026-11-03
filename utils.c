#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void strcat_double(char *restrict dest, double n) {
    const size_t len = snprintf(NULL, 0, "%g", n) + 1;
    char buf[len];
    buf[len - 1] = '\0';
    snprintf(buf, len, "%g", n);
    strcat(dest, buf);
}

char *tokens_to_string(const gvec_t *tokens, int include_eoi) {
    if (tokens == NULL || gvec_size(tokens) == 0
        || (!include_eoi && gvec_size(tokens) == 1
            && ((token_t *)gvec_ptr_at(tokens, 0))->type == TT_EOI))
        return NULL;

    size_t string_size = 0;
    int eoi_in_tokens = 0;
    const size_t size = gvec_size(tokens);
    const token_t *const arr = (const token_t *)gvec_array(tokens);
    for (size_t i = 0; i < size; i++) {
        switch (arr[i].type) {
            case TT_INVALID:
                string_size += strlen("INVALID");
                break;
            case TT_NUMBER:
                string_size += snprintf(NULL, 0, "%g", arr[i].word);
                break;
            case TT_EOI:
                eoi_in_tokens = 1;
                string_size++;
                break;
            default:
                string_size++;
                break;
        }
    }

    // Add enough space for white spaces between the tokens
    string_size += size - eoi_in_tokens * (include_eoi == 0) * 2;

    printf("len: %zu\n", string_size);

    char *const string = calloc(string_size, sizeof(char));

    for (size_t i = 0; i < size; i++) {
        switch (arr[i].type) {
            case TT_INVALID:
                strcat(string, "INVALID");
                break;
            case TT_OPEN_PARENTHESIS:
                strcat(string, "(");
                break;
            case TT_CLOSE_PARENTHESIS:
                strcat(string, ")");
                break;
            case TT_ADDITION:
                strcat(string, "+");
                break;
            case TT_SUBTRACTION:
                strcat(string, "-");
                break;
            case TT_MULTIPLICATION:
                strcat(string, "*");
                break;
            case TT_DIVISION:
                strcat(string, "/");
                break;
            case TT_POWER:
                strcat(string, "^");
                break;
            case TT_NUMBER:
                strcat_double(string, arr[i].word);
                break;
            case TT_EOI:
                if (include_eoi)
                    strcat(string, "$");
                break;
        }

        if (i < size - 1)
            strcat(string, " ");
    }

    return string;
}
