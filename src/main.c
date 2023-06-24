#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "elex.h"
#include "einterpreter.h"
#include "eerror.h"

eString read_file(eArena *arena, const char *path)
{
    FILE *fp = fopen(path, "rb");
    if(!fp)
    {
        fprintf(stderr, "Failed to open file: %s\n", path);

        return (eString) {.ptr = NULL, .len = 0};
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    eString txt = e_string_alloc(arena, len);
    if(!txt.ptr)
    {
        fprintf(stderr, "Failed to read file: %s\n", path);

        return (eString) {.ptr = NULL, .len = 0};
    }

    fread(txt.ptr, 1, len, fp);

    fclose(fp);

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

    eArena arena = e_arena_new(2048);

    eString txt = read_file(&arena, argv[1]);
    if(!txt.ptr)
    {
        return -1;
    }

    eScope global = {
        .variables = NULL
    };

    eListNode *tokens = e_lex(&arena, txt);
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

    eParser parser = e_parser_new(&arena, tokens, txt);

    eASTNode *expr = e_parse_statement(&arena, &parser);
    if(expr == NULL)
    {
        fprintf(stderr, "Parser error in line %ld: %s\n", e_get_error_line(), e_get_error());

        return -3;
    }

    while(expr->tag != AST_EOF)
    {
        e_evaluate(&arena, expr, &global);

        expr = e_parse_statement(&arena, &parser);

        if(expr == NULL)
        {
            fprintf(stderr, "Parser error in line %ld: %s\n", e_get_error_line(), e_get_error());

            return -3;
        }
    }

    e_arena_free(&arena);

    return 0;
}

