#pragma once

#include <stddef.h>

typedef struct earenaregion eArenaRegion;

struct earenaregion
{
    void *ptr;

    size_t size;
    size_t used;

    eArenaRegion *next;
};

typedef struct
{
    eArenaRegion *regions, *current;
} eArena;

eArena e_arena_new(size_t size);

void e_arena_free(eArena *arena);

void *e_arena_alloc(eArena *arena, size_t size);
