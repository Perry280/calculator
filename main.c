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
    err_msg_t err = {0};
    char *aux = NULL;

    switch (tokenize(input, &tokens, &err)) {
        case L_OK:
            aux = tokens_to_string(tokens, 1);
            printf("Tokens: %s\n", aux);
            free(aux);
            aux = NULL;

            switch (recursive_descent_parser(tokens, &err)) {
                case P_OK:
                    printf("Valid syntax.\n");
                    break;
                case P_EINVAL:
                    fprintf(stderr, "ERROR: Invalid argument.\n");
                    break;
                case P_ENOMEM:
                    fprintf(stderr, "ERROR: malloc failed.\n");
                    break;
                case P_SYNERR:
                    aux = format_err_msg(input, err);
                    fprintf(stderr, "\n%s\n\n", aux);
                    free(aux);
                    aux = NULL;
                    break;
            }
            break;
        case L_EINVAL:
            fprintf(stderr, "ERROR: Empty input.\n");
            break;
        case L_ENOMEM:
            fprintf(stderr, "ERROR: malloc failed.\n");
            break;
        case L_ERROR:
            aux = format_err_msg(input, err);
            fprintf(stderr, "\n%s\n\n", aux);
            free(aux);
            aux = NULL;
            break;
    }

    if (argc > 2)
        free(input);
    gvec_free(tokens);

    return 0;
}
