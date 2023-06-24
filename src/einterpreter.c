#include "einterpreter.h"
#include "ebuiltin.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef eValue(* BuiltinFunction)(eArena *arena, eScope *scope, eListNode *arguments);

typedef struct
{
    const char *identifier;

    BuiltinFunction func;

    size_t num_arguments;
} FunctionMap;

static FunctionMap builtin_functions[] = {
    {.identifier = "print", .func = __e_print, .num_arguments = 1},
    {.identifier = "exit", .func = __e_exit, .num_arguments = 1}
};

eValue e_evaluate(eArena *arena, eASTNode *node, eScope *scope)
{
    switch(node->tag)
    {
    case AST_NUMERIC_LITERAL: {
        return (eValue) {
            .type = VT_INT,
            .integer = node->numeric_literal.value
        };
    }

    case AST_STRING_LITERAL: {
        size_t len = strlen(node->string_literal.value);
        char *tmp = e_arena_alloc(arena, len + 1);
        memcpy(tmp, node->string_literal.value, len);
        tmp[len] = '\0';

        return (eValue) {
            .type = VT_STRING,
            .string = tmp
        };
    }

    case AST_IDENTIFIER: {
        eValue value = e_get_value(node->identifier, scope);
        if(value.type == VT_INVALID)
        {
            fprintf(stderr, "Unknown identifier\n");

            exit(1);
        }

        return value;
    }

    case AST_ARITHMETIC: {
        eValue lhs = e_evaluate(arena, node->arithmetic.lhs, scope);
        eValue rhs = e_evaluate(arena, node->arithmetic.rhs, scope);

        switch(node->arithmetic.op)
        {
        case OP_ADD:
            return (eValue) {
                .type = VT_INT,
                .integer = lhs.integer + rhs.integer
            };

        case OP_SUB:
            return (eValue) {
                .type = VT_INT,
                .integer = lhs.integer - rhs.integer
            };

        case OP_MUL:
            return (eValue) {
                .type = VT_INT,
                .integer = lhs.integer * rhs.integer
            };

        case OP_DIV:
            return (eValue) {
                .type = VT_INT,
                .integer = lhs.integer / rhs.integer
            };

        case OP_MOD:
            return (eValue) {
                .type = VT_INT,
                .integer = lhs.integer % rhs.integer
            };

        default:
            return (eValue) {.type = VT_INVALID};
        }
    }

    case AST_DECLARATION: {
        e_declare(arena, node->declaration, scope);

        return (eValue) {.type = VT_INVALID};
    }

    case AST_ASSIGNMENT: {
        e_assign(arena, node->assignment, scope);

        return (eValue) {.type = VT_INVALID};
    }

    case AST_FUNCTION_CALL: {
        return e_call(arena, node->function_call, scope);
    }

    default: {
        return (eValue) {.type = VT_INVALID};
    }
    }
}

eValue e_call(eArena *arena, eASTFunctionCall call, eScope *scope)
{
    for(size_t i = 0; i < sizeof(builtin_functions) / sizeof(FunctionMap); i++)
    {
        if(strcmp(call.identifier, builtin_functions[i].identifier) == 0)
        {
            if(e_list_len(call.arguments) != builtin_functions[i].num_arguments)
            {
                fprintf(stderr, "Wrong amount of arguments provided\n");

                exit(1);
            }

            return builtin_functions[i].func(arena, scope, call.arguments);
        }
    }

    return (eValue) {.type = VT_INVALID};
}

void e_declare(eArena *arena, eASTDeclaration declaration, eScope *scope)
{
    eListNode *current = scope->variables;
    while(current != NULL)
    {
        eVariable *var = (eVariable *) current->data;
        if(strcmp(var->identifier, declaration.identifier) == 0)
        {
            fprintf(stderr, "A variable with that name already exists\n");

            exit(1);
        }
    
        current = current->next;
    }

    size_t len = strlen(declaration.identifier);

    char *tmp = e_arena_alloc(arena, len + 1);
    memcpy(tmp, declaration.identifier, len);
    tmp[len] = '\0';

    e_list_push(arena, &scope->variables, &(eVariable) {
        .identifier = tmp,
        .value = e_evaluate(arena, declaration.init, scope),
        .type = declaration.type
    }, sizeof(eVariable));
}

void e_assign(eArena *arena, eASTAssignment assignment, eScope *scope)
{
    eListNode *current_var = scope->variables;
    while(current_var != NULL)
    {
        eVariable *var = (eVariable *) current_var->data;
        if(strcmp(var->identifier, assignment.identifier) == 0)
        {
            if(var->type == AT_CONST)
            {
                fprintf(stderr, "Cannot reassign a constant variable\n");

                exit(1);
            }

            var->value = e_evaluate(arena, assignment.init, scope);

            return;
        }

        current_var = current_var->next;
    }
}

eValue e_get_value(const char *identifier, eScope *scope)
{
    eListNode *current = scope->variables;
    while(current != NULL)
    {
        eVariable *var = (eVariable *) current->data;
        if(strcmp(var->identifier, identifier) == 0)
        {
            return var->value;
        }

        current = current->next;
    }

    return (eValue) {.type = VT_INVALID};
}

/*
void e_free_scope(eScope *self)
{
    eListNode *current = self->variables;
    while(current != NULL)
    {
        e_free_variable(current->data);

        current = current->next;
    }

    e_list_free(&self->variables);
}
*/

/*
void e_free_variable(eVariable *self)
{
    free(self->identifier);
}
*/
