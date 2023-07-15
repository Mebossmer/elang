#include "eio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

eString e_read_file(eArena *arena, eString path)
{
    eArena tmp = e_arena_new(64);
    char *str = e_arena_alloc(arena, path.len + 1);
    memcpy(str, path.ptr, path.len);
    str[path.len] = '\0';

    FILE *fp = fopen(str, "rb");
    if(!fp)
    {
        fprintf(stderr, "Failed to open file: %s\n", str);
        exit(-1);
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    eString txt = e_string_alloc(arena, len + 1);
    if(!txt.ptr)
    {
        fprintf(stderr, "Failed to read file: %s\n", str);
        exit(-1);
    }

    fread(txt.ptr, 1, len, fp);

    fclose(fp);

    e_arena_free(&tmp);

    return txt;
}
