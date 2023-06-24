#pragma once

#include <stddef.h>

typedef struct
{
    void *base;
    
    size_t size;
    size_t used;
} eArena;

eArena e_arena_new(size_t size);

void e_arena_free(eArena *arena);

void *e_arena_alloc(eArena *arena, size_t size);

void e_arena_resize(eArena *arena, size_t size);
