#include "earena.h"
#include <stdio.h>
#include <stdlib.h>

static eArenaRegion *region_new(size_t size)
{
    eArenaRegion *region = calloc(1, sizeof(eArenaRegion));
    region->ptr = calloc(size, 1);
    region->size = size;
    region->used = 0;
    region->next = NULL;

    return region;
}

static void region_free(eArenaRegion *region)
{
    free(region->ptr);
    free(region);
}

eArena e_arena_new(size_t size)
{
    eArenaRegion *region = region_new(size);

    return (eArena) {
        .regions = region,
        .current = region
    };
}

void e_arena_free(eArena *arena)
{
    eArenaRegion *current = arena->regions;
    while(current != NULL)
    {
        eArenaRegion *tmp = current->next;

        region_free(current);

        current = tmp;
    }
}

void *e_arena_alloc(eArena *arena, size_t size)
{
    if(arena->current->used + size >= arena->current->size)
    {
        if(size >= arena->regions->size)
        {
            arena->regions->next = region_new(size);
        }
        else
        {
            arena->regions->next = region_new(arena->regions->size);
        }

        arena->current = arena->regions->next;
    }

    void *ptr = arena->current->ptr + arena->current->used;
    arena->current->used += size;

    return ptr;
}
