#include "effi.h"
#include "eerror.h"
#include <dlfcn.h>
#include <string.h>

char *c_str(eArena *arena, eString str)
{
    char *tmp = e_arena_alloc(arena, str.len + 1);
    memcpy(tmp, str.ptr, str.len);
    tmp[str.len] = '\0';

    return tmp;
}

// TODO: make this cross-platform
eResult e_ffi_call(eString name, eString lib, eArena *arena, eScope *scope, eListNode *arguments)
{
    eArena tmp = e_arena_new(32);
    char *path = c_str(&tmp, lib);

    void *dl = dlopen(path, RTLD_NOW);
    if(!dl)
    {
        THROW_ERROR(RUNTIME_ERROR, "failed to open library\n", 0l);
    }

    char *fn_name = c_str(&tmp, name);
    eFunctionPtr ptr = dlsym(dl, fn_name);
    if(!ptr)
    {
        THROW_ERROR(RUNTIME_ERROR, "failed to retrieve function pointer\n", 0l);
    }

    eResult result = ptr(arena, scope, arguments);

    e_arena_free(&tmp);

    return result;
}