#include "earena.h"
#include <stdio.h>
#include <stdlib.h>

eArena e_arena_new(size_t size)
{
    void *ptr = calloc(size, 1);

    return (eArena) {
        .base = ptr,
        .size = size,
        .used = 0
    };
}

void e_arena_free(eArena *arena)
{
    free(arena->base);
}

void *e_arena_alloc(eArena *arena, size_t size)
{
    if(arena->used + size >= arena->size)
    {
        // e_arena_resize(arena, arena->used * 2);
        fprintf(stderr, "Arena overflowed: %ld bytes of %ld bytes\n", arena->used + size, arena->size);

        exit(-1);
    }

    void *ptr = arena->base + arena->used;
    arena->used += size;

    return ptr;
}

/*
void e_arena_resize(eArena *arena, size_t size)
{
    void *new_ptr = realloc(arena->base, size);
    if(new_ptr == NULL)
    {
        fprintf(stderr, "Failed to resize an arena\n");

        exit(-1);
    }

    arena->base = new_ptr;

    arena->size = size;
}
*/
