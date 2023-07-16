#include "io.h"
#include <stdio.h>
#include <stdlib.h>

static void println(eString string)
{
    for(size_t i = 0; i < string.len; i++)
    {
        putc(string.ptr[i], stdout);
    }

    putc('\n', stdout);
}

eResult print(eArena *arena, eScope *scope, eListNode *arguments)
{
    eResult result = e_evaluate(arena, E_LIST_AT(arguments, 0, eASTNode *), scope);

    switch(result.value.type)
    {
    case VT_INT:
        printf("%d\n", result.value.integer);

        break;

    case VT_STRING:
        println(result.value.string);

        break;

    case VT_BOOL:
        switch(result.value.boolean)
        {
        case true:
            printf("true\n");

            break;
        
        case false:
            printf("false\n");

            break;
        }
    }

    return (eResult) {.value = {0}, .is_void = true};
}

eResult quit(eArena *arena, eScope *scope, eListNode *arguments)
{
    eResult result = e_evaluate(arena, E_LIST_AT(arguments, 0, eASTNode *), scope);

    exit(result.value.integer);
}
