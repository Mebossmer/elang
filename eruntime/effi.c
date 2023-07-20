#include "effi.h"
#include "eerror.h"
#include <dlfcn.h>
#include <string.h>

typedef eFunctionDef *(* eModuleInitializer)(size_t *num_functions);

char *c_str(eArena *arena, eString str)
{
    char *tmp = e_arena_alloc(arena, str.len + 1);
    memcpy(tmp, str.ptr, str.len);
    tmp[str.len] = '\0';

    return tmp;
}

// TODO: make this cross-platform
eResult e_ffi_call(eString name, eString lib, eArena *arena, eScope *scope, eStack *arguments)
{
    eArena tmp = e_arena_new(32);
    char *path = c_str(&tmp, lib);

    void *dl = dlopen(path, RTLD_NOW);
    if(!dl)
    {
        THROW_ERROR(RUNTIME_ERROR, "failed to open library\n", 0l);
    }

    eModuleInitializer initializer = dlsym(dl, "e_mod_init");
    if(!initializer)
    {
        THROW_ERROR(RUNTIME_ERROR, "failed to retrieve module init function\n", 0l);
    }

    size_t num_functions = 0;
    eFunctionDef *fun_table = initializer(&num_functions);

    for(size_t i = 0; i < num_functions; i++)
    {
        eFunctionDef current = fun_table[i];

        if(e_string_compare(current.name, name))
        {
            if(current.num_args != e_stack_len(arguments))
            {
                THROW_ERROR(RUNTIME_ERROR, "wrong amount of arguments provided\n", 0l);
            }

            e_arena_free(&tmp);

            return current.ptr(arena, scope, arguments);
        }
    }

    e_arena_free(&tmp);

    THROW_ERROR(RUNTIME_ERROR, "no functions found with that name\n", 0l);
}