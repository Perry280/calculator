#include "lexer.h"
#include "parser.h"
#include "types.h"
#include "utils.h"
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

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "String to evaluate not found.\n");
        return EXIT_FAILURE;
    }

    char *input = argc == 2 ? argv[1] : concat_input_expression(argc, argv);
    printf("Input: %s\n", input);

    gvec_t *tokens = NULL;
    char *msg = NULL;
    char *tokens_str = NULL;
    switch (tokenize(input, &tokens)) {
        case L_OK:
            tokens_str = tokens_to_string(tokens, 1);
            printf("Tokens: %s\n", tokens_str);
            free(tokens_str);
            tokens_str = NULL;
            switch (recursive_descent_parser(tokens, &msg)) {
                case P_OK:
                    printf("Valid syntax.\n");
                    break;
                case P_ENOMEM:
                    fprintf(stderr, "malloc falied.\n");
                    break;
                case P_EINVAL:
                    fprintf(stderr, "Invalid argument.\n");
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
