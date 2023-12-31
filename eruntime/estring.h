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

eString e_string_combine(eArena *arena, eString a, eString b);

eString e_string_combine_member(eArena *arena, eString a, eString b);

/**
 * Gets the filename a path string points to
*/
eString e_string_slice_file_name(eString str);

/**
 * Gets the path without the filename the path string points to
*/
eString e_string_slice_file_path(eString str);

bool e_string_compare(eString a, eString b);

void e_string_print(eString msg);
