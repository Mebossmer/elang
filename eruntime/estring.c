#include "estring.h"
#include <string.h>
#include <stdio.h>

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

eString e_string_combine(eArena *arena, eString a, eString b)
{
    eString result = e_string_alloc(arena, a.len + b.len);
    memcpy(result.ptr, a.ptr, a.len);
    memcpy(result.ptr + a.len, b.ptr, b.len);

    return result;
}

eString e_string_combine_member(eArena *arena, eString a, eString b)
{
    eString result = e_string_alloc(arena, a.len + b.len + 1);
    memcpy(result.ptr, a.ptr, a.len);
    memcpy(result.ptr + a.len + 1, b.ptr, b.len);
    result.ptr[a.len] = '.';

    return result;
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

void e_string_print(eString msg)
{
    for(size_t i = 0; i < msg.len; i++)
    {
        putc(msg.ptr[i], stdout);
    }
}
