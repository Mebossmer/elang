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

eResult print(eArena *arena, eScope *scope, eStack *arguments)
{
    eValue value = E_STACK_POP(arguments, eValue);

    switch(value.type)
    {
    case VT_INT:
        printf("%d\n", value.integer);

        break;

    case VT_STRING:
        println(value.string);

        break;

    case VT_BOOL:
        switch(value.boolean)
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

eResult quit(eArena *arena, eScope *scope, eStack *arguments)
{
    eValue value = E_STACK_POP(arguments, eValue);

    exit(value.integer);
}
