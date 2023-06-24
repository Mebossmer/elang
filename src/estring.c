#include "estring.h"
#include <string.h>

eString e_string_new(eArena *arena, const char *text)
{
    size_t len = strlen(text);
    char *ptr = e_arena_alloc(arena, len);
    memcpy(ptr, text, len);

    return (eString) {
        .ptr = ptr,
        .len = len
    };
}

eString e_string_alloc(eArena *arena, size_t len)
{
    return (eString) {
        .ptr = e_arena_alloc(arena, len),
        .len = len
    };
}

eString e_string_slice(eString string, size_t index, size_t len)
{
    return (eString) {
        .ptr = string.ptr + index,
        .len = len
    };
}

bool e_string_compare(eString a, eString b)
{
    if(a.len != b.len)
    {
        return false;
    }

    for(size_t i = 0; i < a.len; i++)
    {
        if(a.ptr[i] != b.ptr[i])
        {
            return false;
        }
    }

    return true;
}
