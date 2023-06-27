#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <elex.h>
#include <einterpreter.h>
#include <eerror.h>

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

    eString txt = e_string_alloc(arena, len + 1);
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

    eScope global = e_scope_new(NULL);

    eListNode *tokens = e_lex(&arena, txt);

    eParser parser = e_parser_new(&arena, tokens, txt);

    eASTNode *expr = e_parse_statement(&arena, &parser);

    while(expr->tag != AST_EOF)
    {
        eResult value = e_evaluate(&global.allocator, expr, &global);

        expr = e_parse_statement(&arena, &parser);
    }

    e_scope_free(&global);
    e_arena_free(&arena);

    return 0;
}

