#include "ebuiltin.h"
#include <stdio.h>
#include <stdlib.h>

eValue __e_print(eScope *scope, eListNode *arguments)
{
    eValue value = e_evaluate(E_LIST_AT(arguments, 0, eASTNode *), scope);

    switch(value.type)
    {
    case VT_INT:
        printf("%d\n", value.integer);

        break;

    case VT_STRING:
        printf("%s\n", value.string);

        break;
    }

    return (eValue) {.type = VT_INVALID};
}

eValue __e_exit(eScope *scope, eListNode *arguments)
{
    eValue value = e_evaluate(E_LIST_AT(arguments, 0, eASTNode *), scope);

    exit(value.integer);
}
