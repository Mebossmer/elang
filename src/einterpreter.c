#include "einterpreter.h"
#include "ebuiltin.h"
#include "eerror.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef eValue(* BuiltinFunction)(eArena *arena, eScope *scope, eListNode *arguments);

typedef struct
{
    eString identifier;

    BuiltinFunction func;

    size_t num_arguments;
} FunctionMap;

static FunctionMap builtin_functions[] = {
    {.identifier = {.ptr = "print", .len = 5}, .func = __e_print, .num_arguments = 1},
    {.identifier = {.ptr = "exit", .len = 4}, .func = __e_exit, .num_arguments = 1}
};

eScope e_scope_new(eScope *parent)
{
    return (eScope) {
        .allocator = e_arena_new(2048),
        .parent = parent,
        .functions = NULL,
        .variables = NULL
    };
}

void e_scope_free(eScope *scope)
{
    e_arena_free(&scope->allocator);
}

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
        return (eValue) {
            .type = VT_STRING,
            .string = node->string_literal.value
        };
    }

    case AST_IDENTIFIER: {
        eValue value = e_get_value(node->identifier, scope);
        if(value.type == VT_ERROR)
        {
            e_errcode = ERR_UNKNOWN_IDENTIFIER;
            e_errline = 0; // TODO

            return (eValue) {.type = VT_ERROR};
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
        if(!e_declare(arena, node->declaration, scope))
        {
            return (eValue) {.type = VT_ERROR};
        }

        return (eValue) {.type = VT_INVALID};
    }

    case AST_ASSIGNMENT: {
        if(!e_assign(arena, node->assignment, scope))
        {
            return (eValue) {.type = VT_ERROR};
        }

        return (eValue) {.type = VT_INVALID};
    }

    case AST_FUNCTION_CALL: {
        return e_call(arena, node->function_call, scope);
    }

    case AST_IF_STATEMENT: {
        eValue condition = e_evaluate(arena, node->if_statement.condition, scope);

        if(condition.boolean)
        {
            e_evaluate_body(arena, node->if_statement.body, scope);
        }

        return (eValue) {.type = VT_INVALID};
    }

    case AST_WHILE_LOOP: {
        eValue condition = e_evaluate(arena, node->while_loop.condition, scope);

        while(condition.boolean)
        {
            e_evaluate_body(arena, node->while_loop.body, scope);

            condition = e_evaluate(arena, node->while_loop.condition, scope);
        }

        return (eValue) {.type = VT_INVALID};
    }

    case AST_BOOL_LITERAL: {
        return (eValue) {.type = VT_BOOL, .boolean = node->bool_literal.value};
    }

    case AST_CONDITION: {
        eValue lhs = e_evaluate(arena, node->condition.lhs, scope);
        eValue rhs = e_evaluate(arena, node->condition.rhs, scope);

        switch(node->condition.op)
        {
        case BOP_AND:
            return (eValue) {
                .type = VT_BOOL,
                .boolean = lhs.boolean && rhs.boolean
            };

        case BOP_OR:
            return (eValue) {
                .type = VT_BOOL,
                .boolean = lhs.boolean || rhs.boolean
            };

        case BOP_IS_EQUAL:
            return (eValue) {
                .type = VT_BOOL,
                .boolean = lhs.integer == rhs.integer // TODO: very bad
            };

        case BOP_IS_LESS:
            return (eValue) {
                .type = VT_BOOL,
                .boolean = lhs.integer < rhs.integer // TODO: also very bad
            };

        case BOP_IS_GREATER:
            return (eValue) {
                .type = VT_BOOL,
                .boolean = lhs.integer > rhs.integer // TODO: pathetic
            };

        default:
            return (eValue) {.type = VT_INVALID};
        }
    }

    case AST_FUNCTION_DECL:
        e_declare_function(arena, node->function_decl, scope);

        return (eValue) {.type = VT_INVALID};

    default: {
        return (eValue) {.type = VT_INVALID};
    }
    }
}

void e_evaluate_body(eArena *arena, eListNode *body, eScope *scope)
{
    eListNode *current = body;
    while(current != NULL)
    {
        e_evaluate(arena, (eASTNode *) current->data, scope);

        current = current->next;
    }
}

static eASTFunctionDecl *get_function(eString identifier, eScope *scope)
{
    eScope *current = scope;
    while(current != NULL)
    {
        eListNode *current_function = current->functions;
        while(current_function != NULL)
        {
            eASTFunctionDecl *function = (eASTFunctionDecl *) current_function->data;
            if(e_string_compare(function->identifier, identifier))
            {
                return function;
            }

            current_function = current_function->next;
        }

        current = current->parent;
    }

    return NULL;
}

static FunctionMap get_builtin_function(eString identifier)
{
    for(size_t i = 0; i < sizeof(builtin_functions) / sizeof(FunctionMap); i++)
    {
        if(e_string_compare(identifier, builtin_functions[i].identifier))
        {
            return builtin_functions[i];
        }
    }

    return (FunctionMap) {.func = NULL, .identifier = {0}, .num_arguments = 0};
}

eValue e_call(eArena *arena, eASTFunctionCall call, eScope *scope)
{
    /*
    eListNode *current = scope->functions;
    while(current != NULL)
    {
        eASTFunctionDecl *decl = (eASTFunctionDecl *) current->data;
        if(e_string_compare(decl->identifier, call.identifier))
        {
            // Create new scope and execute function

            eScope function_scope = e_scope_new(scope);

            e_evaluate_body(arena, decl->body, scope);

            e_scope_free(&function_scope);

            return (eValue) {.type = VT_INVALID};
        }

        current = current->next;
    }
    */

    eASTFunctionDecl *function = get_function(call.identifier, scope);
    if(function != NULL)
    {
        eScope function_scope = e_scope_new(scope);

        eListNode *current_arg = call.arguments;
        eListNode *current_param = function->params;
        while(current_arg != NULL)
        {
            eString *identifier = (eString *) current_param->data;

            e_declare(&function_scope.allocator, (eASTDeclaration) {
                .identifier = *identifier,
                .init = (eASTNode *) current_arg->data,
                .type = AT_VAR // TODO
            }, &function_scope);

            current_arg = current_arg->next;
            current_param = current_param->next;
        }

        e_evaluate_body(arena, function->body, &function_scope);

        e_scope_free(&function_scope);

        return (eValue) {.type = VT_INVALID};
    }

    /*
    for(size_t i = 0; i < sizeof(builtin_functions) / sizeof(FunctionMap); i++)
    {
        if(e_string_compare(call.identifier, builtin_functions[i].identifier))
        {
            if(e_list_len(call.arguments) != builtin_functions[i].num_arguments)
            {
                e_errcode = ERR_ARGUMENT_COUNT;
                e_errline = 0; // TODO

                return (eValue) {.type = VT_ERROR};
            }

            return builtin_functions[i].func(arena, scope, call.arguments);
        }
    }
    */

    FunctionMap built_in = get_builtin_function(call.identifier);
    if(built_in.func != NULL)
    {
        if(e_list_len(call.arguments) != built_in.num_arguments)
        {
            e_errcode = ERR_ARGUMENT_COUNT;
            e_errline = 0; // TODO

            return (eValue) {.type = VT_ERROR};
        }

        return built_in.func(arena, scope, call.arguments);
    }

    e_errcode = ERR_UNKNOWN_IDENTIFIER;
    e_errline = 0; // TODO

    return (eValue) {.type = VT_ERROR};
}

bool e_declare(eArena *arena, eASTDeclaration declaration, eScope *scope)
{
    eScope *current_scope = scope;
    while(current_scope != NULL)
    {
        eListNode *current = current_scope->variables;
        while(current != NULL)
        {
            eVariable *var = (eVariable *) current->data;
            if(e_string_compare(var->identifier, declaration.identifier))
            {
                e_errcode = ERR_NAME_CONFLICT;
                e_errline = 0; // TODO

                return false;
            }
        
            current = current->next;
        }

        current_scope = current_scope->parent;
    }

    eValue value = e_evaluate(arena, declaration.init, scope);
    if(value.type == VT_ERROR)
    {
        return false;
    }

    e_list_push(arena, &scope->variables, &(eVariable) {
        .identifier = declaration.identifier,
        .value = value,
        .type = declaration.type
    }, sizeof(eVariable));

    return true;
}

bool e_declare_function(eArena *arena, eASTFunctionDecl declaration, eScope *scope)
{
    eScope *current_scope = scope;
    while(current_scope != NULL)
    {
        eListNode *current = scope->functions;
        while(current != NULL)
        {
            eASTFunctionDecl *decl = (eASTFunctionDecl *) current->data;
            if(e_string_compare(decl->identifier, declaration.identifier))
            {
                e_errcode = ERR_NAME_CONFLICT;
                e_errline = 0; // TODO

                return false;
            }

            current = current->next;
        }

        current_scope = current_scope->parent;
    }

    e_list_push(arena, &scope->functions, &declaration, sizeof(eASTFunctionDecl));

    return true;
}

bool e_assign(eArena *arena, eASTAssignment assignment, eScope *scope)
{
    eScope *current_scope = scope;
    while(current_scope != NULL)
    {
        eListNode *current_var = current_scope->variables;
        while(current_var != NULL)
        {
            eVariable *var = (eVariable *) current_var->data;
            if(e_string_compare(var->identifier, assignment.identifier))
            {
                if(var->type == AT_CONST)
                {
                    e_errcode = ERR_CONST_REASSIGNMENT;
                    e_errline = 0;
                
                    return false;
                }

                var->value = e_evaluate(arena, assignment.init, scope);

                return true;
            }

            current_var = current_var->next;
        }

        current_scope = current_scope->parent;
    }

    return false;
}

eValue e_get_value(eString identifier, eScope *scope)
{
    eScope *current_scope = scope;
    while(current_scope != NULL)
    {
        eListNode *current = current_scope->variables;
        while(current != NULL)
        {
            eVariable *var = (eVariable *) current->data;
            if(e_string_compare(var->identifier, identifier))
            {
                return var->value;
            }

            current = current->next;
        }

        current_scope = current_scope->parent;
    }

    return (eValue) {.type = VT_ERROR};
}
