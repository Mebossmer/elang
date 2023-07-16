#include "estack.h"
#include "eerror.h"
#include <string.h>

eStack e_stack_new(eArena *arena, size_t size, size_t item_size)
{
    eStack stack = {0};

    stack.base = e_arena_alloc(arena, size);
    stack.ptr = stack.base;

    stack.used = 0;
    stack.size = size;
    stack.item_size = item_size;

    return stack;
}

void e_stack_push(eArena *arena, eStack *stack, void *data)
{
    if(stack->used + stack->item_size >= stack->size)
    {
        // THROW_ERROR(RUNTIME_ERROR, "stack overflow", 0l);

        stack->base = realloc(stack->base, stack->size * 2 + stack->item_size);
        stack->ptr = stack->base + stack->used;
        if(!stack->base)
        {
            THROW_ERROR(RUNTIME_ERROR, "failed to realloc stack", 0l);
        }
    }

    stack->used += stack->item_size;

    memcpy(stack->ptr, data, stack->item_size);
    stack->ptr += stack->item_size;
}

void *e_stack_pop(eStack *stack)
{
    stack->ptr -= stack->item_size;
    if(stack->ptr < stack->base)
    {
        THROW_ERROR(RUNTIME_ERROR, "stack error", 0l);
    }

    return stack->ptr;
}

size_t e_stack_len(eStack *stack)
{
    return stack->used / stack->item_size;
}
