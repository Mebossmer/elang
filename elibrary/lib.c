#include <einterpreter.h>
#include <stdio.h>
#include <stdlib.h>
#include <effi.h>

static void println(eString string)
{
    for(size_t i = 0; i < string.len; i++)
    {
        putc(string.ptr[i], stdout);
    }

    putc('\n', stdout);
}

static eResult io_print(eArena *arena, eScope *scope, eStack *arguments)
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

static eResult io_exit(eArena *arena, eScope *scope, eStack *arguments)
{
    eValue value = E_STACK_POP(arguments, eValue);

    exit(value.integer);
}

static eFunctionDef function_table[] =
{
    {
        .name = (eString) {.ptr = "print", .len = 5},
        .ptr = io_print,
        .num_args = 1
    },
    {
        .name = (eString) {.ptr = "exit", .len = 4},
        .ptr = io_exit,
        .num_args = 1
    }
};

eFunctionDef *e_mod_init(size_t *num_functions)
{
    *num_functions = sizeof(function_table) / sizeof(function_table[0]);

    return function_table;
}
