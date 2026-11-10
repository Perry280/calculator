#include "lexer.h"
#include "parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *concat_input_expression(int argc, char *argv[]) {
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

static void parse(const char *input, const gvec_t *tokens) {
    err_msg_t err = {0};
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
            print_err_msg(input, err);
            break;
    }
}

static void evaluate(const char *input) {
    gvec_t *tokens = NULL;
    err_msg_t err = {0};

    switch (tokenize(input, &tokens, &err)) {
        case L_OK:
            // temp = tokens_to_string(tokens, 1);
            // printf("Tokens: %s\n", temp);
            // free(temp);
            // temp = NULL;

            parse(input, tokens);
            break;
        case L_EINVAL:
            fprintf(stderr, "ERROR: Empty input.\n");
            break;
        case L_ENOMEM:
            fprintf(stderr, "ERROR: malloc failed.\n");
            break;
        case L_ERROR:
            print_err_msg(input, err);
            break;
    }

    gvec_free(tokens);
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "String to evaluate not found.\n");
        return EXIT_FAILURE;
    }

    char *input = argc == 2 ? argv[1] : concat_input_expression(argc, argv);
    // printf("Input: %s\n", input);

    evaluate(input);

    if (argc > 2)
        free(input);

    return 0;
}
