#pragma once

#include "earena.h"
#include <stddef.h>

#define E_STACK_POP(_stack, _type) (*(_type *) e_stack_pop(_stack))

typedef struct
{
    void *base, *ptr;

    size_t used, size, item_size;
} eStack;

eStack e_stack_new(eArena *arena, size_t size, size_t item_size);

void e_stack_push(eArena *arena, eStack *stack, void *data);

void *e_stack_pop(eStack *stack);

size_t e_stack_len(eStack *stack);
