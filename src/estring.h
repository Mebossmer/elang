#pragma once

#include "earena.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    char *ptr;

    size_t len;
} eString;

eString e_string_new(eArena *arena, const char *text);

eString e_string_alloc(eArena *arena, size_t len);

eString e_string_slice(eString string, size_t index, size_t len);

bool e_string_compare(eString a, eString b);

void e_string_print(eString msg);
