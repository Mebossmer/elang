#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "elex.h"
#include "einterpreter.h"
#include "eerror.h"

char *read_file(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if(!fp)
    {
        fprintf(stderr, "Failed to open file: %s\n", path);

        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    char *txt = malloc(len);
    if(!txt)
    {
        fprintf(stderr, "Failed to read file: %s\n", path);

        return NULL;
    }

    fread(txt, 1, len, fp);

    fclose(fp);

    txt[len] = '\0';

    return txt;
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Usage:\n");
        printf("elang <filename>\n");

        return 0;
    }

    char *txt = read_file(argv[1]);
    if(!txt)
    {
        return -1;
    }

    eScope global = {
        .variables = NULL
    };

    eListNode *tokens = e_lex(txt);
    eListNode *current = tokens;
    while(current != NULL)
    {
        eToken *tk = (eToken *) current->data;
        if(tk->tag == ETK_UNKNOWN)
        {
            fprintf(stderr, "Lexer error in line %ld: %s\n", e_get_error_line(), e_get_error());

            return -2;
        }

        current = current->next;
    }

    eParser parser = e_parser_new(tokens, txt);

    eASTNode *expr = e_parse_statement(&parser);
    if(expr == NULL)
    {
        fprintf(stderr, "Parser error in line %ld: %s\n", e_get_error_line(), e_get_error());

        return -3;
    }

    while(expr->tag != AST_EOF)
    {
        e_evaluate(expr, &global);

        e_ast_free(expr);

        expr = e_parse_statement(&parser);

        if(expr == NULL)
        {
            fprintf(stderr, "Parser error in line %ld: %s\n", e_get_error_line(), e_get_error());

            return -3;
        }
    }

    e_ast_free(expr);

    e_free_scope(&global);

    e_parser_free(&parser);
    e_list_free(&tokens);

    free(txt);

    return 0;
}

