#include "lexer.h"
#include "parser.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *concat_input_expression(int argc, char *argv[]) {
    int args = argc - 1;
    char **strings = argv + 1;
    size_t input_len = 0;

    for (int i = 0; i < args; i++)
        input_len += strlen(strings[i]);

    size_t spaces = (size_t)(argc - 1);
    int is_token = 1;
    char *input = calloc(input_len + spaces + 1, sizeof(char));
    for (int i = 0; i < args;) {
        strcat(input, is_token ? strings[i++] : " ");
        is_token = !is_token;
    }

    return input;
}

void print_tokens(gvec_t *tokens) {
    if (tokens == NULL)
        return;

    token_t *arr = (token_t *)(tokens->array);
    printf("Tokens:");
    for (size_t i = 0; i < tokens->size; i++) {
        switch (arr[i].type) {
            case TT_INVALID:
                printf(" Invalid");
                break;
            case TT_OPEN_PARENTHESIS:
                printf(" (");
                break;
            case TT_CLOSED_PARENTHESIS:
                printf(" )");
                break;
            case TT_ADDITION:
                printf(" +");
                break;
            case TT_SUBTRACTION:
                printf(" -");
                break;
            case TT_MULTIPLICATION:
                printf(" *");
                break;
            case TT_DIVISION:
                printf(" /");
                break;
            case TT_POWER:
                printf(" ^");
                break;
            case TT_NUMBER:
                printf(" %g", arr[i].word);
                break;
            case TT_EOI:
                break;
        }
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "String to evaluate not found.\n");
        return EXIT_FAILURE;
    }

    char *input = argc == 2 ? argv[1] : concat_input_expression(argc, argv);
    printf("Input: %s\n", input);

    gvec_t *tokens = NULL;
    switch (tokenize(input, &tokens)) {
        case L_OK:
            print_tokens(tokens);
            switch (recursive_descent_parser(tokens)) {
                case P_OK:
                    printf("Valid syntax.\n");
                    break;
                case P_ENOMEM:
                    fprintf(stderr, "malloc falied.\n");
                    break;
                case P_SYNERR:
                    fprintf(stderr, "Syntax error.\n");
                    break;
            }
            break;
        case L_EINVAL:
            fprintf(stderr, "Empty input.\n");
            break;
        case L_ENOMEM:
            fprintf(stderr, "malloc failed.\n");
            break;
        case L_ERROR:
            fprintf(stderr, "Invalid input.\n");
            break;
    }

    if (argc > 2)
        free(input);
    gvec_free(tokens);

    return 0;
}
