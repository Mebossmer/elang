#include "earena.h"
#include <stdlib.h>

eArena e_arena_new(size_t size)
{
    return (eArena) {
        .base = malloc(size),
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
    arena->used += size;
    if(arena->used >= arena->size)
    {
        e_arena_resize(arena, (arena->size * 2) + size);
    }

    return arena->base + arena->used;
}

void e_arena_resize(eArena *arena, size_t size)
{
    arena->base = realloc(arena->base, size);
    arena->size = size;
}
