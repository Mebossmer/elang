#include "einterpreter.h"
#include "ebuiltin.h"
#include "eerror.h"
#include "eio.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef eResult(* BuiltinFunction)(eArena *arena, eScope *scope, eListNode *arguments);

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

bool e_exec_file(eString path, eScope *scope)
{
    eString txt = e_read_file(&scope->allocator, path);
    if(!txt.ptr)
    {
        return false;
    }

    eListNode *tokens = e_lex(&scope->allocator, txt);

    eParser parser = e_parser_new(tokens, txt);

    eASTNode *expr = e_parse_statement(&scope->allocator, &parser);

    while(expr->tag != AST_EOF)
    {
        eResult value = e_evaluate(&scope->allocator, expr, scope);

        expr = e_parse_statement(&scope->allocator, &parser);
    }

    return true;
}

eScope e_scope_new(eScope *parent, eASTFunctionDecl *function)
{
    return (eScope) {
        .allocator = e_arena_new(2048),
        .parent = parent,
        .functions = NULL,
        .variables = NULL,
        .function = function
    };
}

void e_scope_free(eScope *scope)
{
    e_arena_free(&scope->allocator);
}

static bool is_equal(eValue a, eValue b)
{
    if(a.type != b.type)
    {
        THROW_ERROR(RUNTIME_ERROR, "cannot compare two values of different types", 0l);
    }

    switch(a.type)
    {
    case VT_INT:
        return a.integer == b.integer;

    case VT_BOOL:
        return a.boolean == b.boolean;

    case VT_STRING:
        return e_string_compare(a.string, b.string);

    default:
        THROW_ERROR(RUNTIME_ERROR, "invalid comparison", 0l);
    }
}

static bool is_less(eValue a, eValue b)
{
    if(a.type != b.type)
    {
        THROW_ERROR(RUNTIME_ERROR, "cannot compare two values of different types", 0l);
    }

    switch(a.type)
    {
    case VT_INT:
        return a.integer < b.integer;

    default:
        THROW_ERROR(RUNTIME_ERROR, "invalid comparison", 0l);
    }
}

static bool is_greater(eValue a, eValue b)
{
    if(a.type != b.type)
    {
        THROW_ERROR(RUNTIME_ERROR, "cannot compare two values of different types", 0l);
    }

    switch(a.type)
    {
    case VT_INT:
        return a.integer > b.integer;

    default:
        THROW_ERROR(RUNTIME_ERROR, "invalid comparison", 0l);
    }
}

eResult e_evaluate(eArena *arena, eASTNode *node, eScope *scope)
{
    switch(node->tag)
    {
    case AST_NUMERIC_LITERAL: {
        return (eResult) {
            .value = {
                .type = VT_INT,
                .integer = node->numeric_literal.value
            },
            .is_void = false,
            .is_return = false
        };
    }

    case AST_STRING_LITERAL: {
        return (eResult) {
            .value = {
                .type = VT_STRING,
                .string = node->string_literal.value
            },
            .is_void = false,
            .is_return = false
        };
    }

    case AST_IDENTIFIER: {
        eValue value = e_get_value(node->identifier, scope);

        return (eResult) {
            .value = value,
            .is_void = false,
            .is_return = false
        };
    }

    case AST_ARITHMETIC: {
        eResult lhs = e_evaluate(arena, node->arithmetic.lhs, scope);
        eResult rhs = e_evaluate(arena, node->arithmetic.rhs, scope);

        switch(node->arithmetic.op)
        {
        case OP_ADD:
            return (eResult) {
                .value = {
                    .type = VT_INT,
                    .integer = lhs.value.integer + rhs.value.integer
                },
                .is_void = false,
                .is_return = false
            };

        case OP_SUB:
            return (eResult) {
                .value = {
                    .type = VT_INT,
                    .integer = lhs.value.integer - rhs.value.integer
                },
                .is_void = false,
                .is_return = false
            };

        case OP_MUL:
            return (eResult) {
                .value = {
                    .type = VT_INT,
                    .integer = lhs.value.integer * rhs.value.integer
                },
                .is_void = false,
                .is_return = false
            };

        case OP_DIV:
            return (eResult) {
                .value = {
                    .type = VT_INT,
                    .integer = lhs.value.integer / rhs.value.integer
                },
                .is_void = false,
                .is_return = false
            };

        case OP_MOD:
            return (eResult) {
                .value = {
                    .type = VT_INT,
                    .integer = lhs.value.integer % rhs.value.integer
                },
                .is_void = false,
                .is_return = false
            };

        default:
            THROW_ERROR(RUNTIME_ERROR, "invalid arithmetic operation", 0l);
        }
    }

    case AST_DECLARATION: {
        e_declare(arena, node->declaration, scope);

        return (eResult) {.value = {0}, .is_void = true, .is_return = false};
    }

    case AST_ASSIGNMENT: {
        e_assign(arena, node->assignment, scope);

        return (eResult) {.value = {0}, .is_void = true, .is_return = false};
    }

    case AST_FUNCTION_CALL: {
        return e_call(arena, node->function_call, scope);
    }

    case AST_IF_STATEMENT: {
        eResult condition = e_evaluate(arena, node->if_statement.condition, scope);

        eResult result = {0};
        if(condition.value.boolean)
        {
            result = e_evaluate_body(arena, node->if_statement.body, scope);
        }
        else if(node->if_statement.else_body != NULL)
        {
            result = e_evaluate_body(arena, node->if_statement.else_body, scope);
        }

        // return (eResult) {.value = {0}, .is_void = true, .is_return = false};
        return result;
    }

    case AST_WHILE_LOOP: {
        eResult condition = e_evaluate(arena, node->while_loop.condition, scope);

        while(condition.value.boolean)
        {
            e_evaluate_body(arena, node->while_loop.body, scope);

            condition = e_evaluate(arena, node->while_loop.condition, scope);
        }

        return (eResult) {.value = {0}, .is_void = true, .is_return = false};
    }

    case AST_BOOL_LITERAL: {
        return (eResult) {
            .value = {
                .type = VT_BOOL,
                .boolean = node->bool_literal.value
            },
            .is_void = false,
            .is_return = false
        };
    }

    case AST_CONDITION: {
        eResult lhs = e_evaluate(arena, node->condition.lhs, scope);
        eResult rhs = e_evaluate(arena, node->condition.rhs, scope);

        switch(node->condition.op)
        {
        case BOP_AND:
            return (eResult) {
                .value = {
                    .type = VT_BOOL,
                    .boolean = lhs.value.boolean && rhs.value.boolean
                },
                .is_void = false,
                .is_return = false
            };

        case BOP_OR:
            return (eResult) {
                .value = {
                    .type = VT_BOOL,
                    .boolean = lhs.value.boolean || rhs.value.boolean
                },
                .is_void = false,
                .is_return = false
            };

        case BOP_IS_EQUAL:
            return (eResult) {
                .value = {
                    .type = VT_BOOL,
                    .boolean = is_equal(lhs.value, rhs.value)
                },
                .is_void = false,
                .is_return = false
            };

        case BOP_IS_LESS:
            return (eResult) {
                .value = {
                    .type = VT_BOOL,
                    .boolean = is_less(lhs.value, rhs.value)
                },
                .is_void = false,
                .is_return = false
            };

        case BOP_IS_GREATER:
            return (eResult) {
                .value = {
                    .type = VT_BOOL,
                    .boolean = is_greater(lhs.value, rhs.value)
                },
                .is_void = false,
                .is_return = false
            };

        default:
            THROW_ERROR(RUNTIME_ERROR, "unknown condition", 0l);
        }
    }

    case AST_FUNCTION_DECL:
        e_declare_function(arena, node->function_decl, scope);

        return (eResult) {.value = {0}, .is_void = true, .is_return = false};

    case AST_RETURN: {
        if(scope->function == NULL)
        {
            THROW_ERROR(RUNTIME_ERROR, "cannot return ouside of function", 0l);
        }

        eResult return_value = e_evaluate(arena, node->return_stmt.arg, scope);
        if(return_value.value.type != scope->function->return_type || scope->function->return_type == VT_VOID)
        {
            THROW_ERROR(RUNTIME_ERROR, "invalid return value", 0l);
        }

        if(return_value.value.type == VT_VOID)
        {
            THROW_ERROR(RUNTIME_ERROR, "cannot return a void type", 0l);
        }

        return (eResult) {.value = return_value.value, .is_void = false, .is_return = true};
    }

    case AST_IMPORT: {
        e_exec_file(e_string_slice(node->import_stmt.path, 1, node->import_stmt.path.len - 2), scope);

        return (eResult) {.value = {0}, .is_void = true, .is_return = false};
    }

    default: {
        THROW_ERROR(RUNTIME_ERROR, "unknown expression", 0l);
    }
    }
}

eResult e_evaluate_body(eArena *arena, eListNode *body, eScope *scope)
{
    eListNode *current = body;
    while(current != NULL)
    {
        eResult result = e_evaluate(arena, (eASTNode *) current->data, scope);
        if(result.is_return)
        {
            return result;
        }

        current = current->next;
    }

    return (eResult) {.value = {0}, .is_void = true, .is_return = false};
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

eResult e_call(eArena *arena, eASTFunctionCall call, eScope *scope)
{
    eASTFunctionDecl *function = get_function(call.identifier, scope);
    if(function != NULL)
    {
        if(e_list_len(call.arguments) != e_list_len(function->params))
        {
            THROW_ERROR(RUNTIME_ERROR, "wrong amount of arguments provided", 0l);
        }

        eScope function_scope = e_scope_new(scope, function);

        eListNode *current_arg = call.arguments;
        eListNode *current_param = function->params;
        while(current_arg != NULL)
        {
            eASTFunctionParam *param = (eASTFunctionParam *) current_param->data;

            e_declare(&function_scope.allocator, (eASTDeclaration) {
                .identifier = param->identifier,
                .init = (eASTNode *) current_arg->data,
                .type = AT_VAR,
                .value_type = param->value_type
            }, &function_scope);

            current_arg = current_arg->next;
            current_param = current_param->next;
        }

        // e_evaluate_body(arena, function->body, &function_scope);

        eListNode *current = function->body;
        while(current != NULL)
        {
            eASTNode *node = (eASTNode *) current->data;
            /*
            // TODO: This is a workaround, please fix
            if(node->tag == AST_RETURN)
            {
                eResult return_value = e_evaluate(arena, node->return_stmt.arg, &function_scope);
                if(return_value.value.type != function->return_type || function->return_type == VT_VOID)
                {
                    THROW_ERROR(RUNTIME_ERROR, "invalid return value", 0l);
                }

                e_scope_free(&function_scope);

                return return_value;
            }
            */

            eResult result = e_evaluate(arena, node, &function_scope);
            if(result.is_return)
            {
                // Return from the function
                e_scope_free(&function_scope);

                return result;
            }

            current = current->next;
        }

        e_scope_free(&function_scope);

        if(function->return_type != VT_VOID)
        {
            THROW_ERROR(RUNTIME_ERROR, "no return statement found inside function", 0l);
        }

        return (eResult) {.value = {0}, .is_void = true, .is_return = false};
    }

    FunctionMap built_in = get_builtin_function(call.identifier);
    if(built_in.func != NULL)
    {
        if(e_list_len(call.arguments) != built_in.num_arguments)
        {
            THROW_ERROR(RUNTIME_ERROR, "invalid amount of arguments", 0l);
        }

        return built_in.func(arena, scope, call.arguments);
    }

    THROW_ERROR(RUNTIME_ERROR, "unknown identifier", 0l);
}

void e_declare(eArena *arena, eASTDeclaration declaration, eScope *scope)
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
                THROW_ERROR(RUNTIME_ERROR, "name conflict", 0l);
            }
        
            current = current->next;
        }

        current_scope = current_scope->parent;
    }

    eResult result = e_evaluate(arena, declaration.init, scope);

    if(result.is_void)
    {
        THROW_ERROR(RUNTIME_ERROR, "cannot assign a void type to a variable", 0l);
    }

    if(result.value.type != declaration.value_type && declaration.value_type != VT_VOID)
    {
        THROW_ERROR(RUNTIME_ERROR, "type conflict", 0l);
    }

    e_list_push(arena, &scope->variables, &(eVariable) {
        .identifier = declaration.identifier,
        .value = result.value,
        .type = declaration.type,
        .value_type = result.value.type
    }, sizeof(eVariable));
}

void e_declare_function(eArena *arena, eASTFunctionDecl declaration, eScope *scope)
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
                THROW_ERROR(RUNTIME_ERROR, "name conflict", 0l);
            }

            current = current->next;
        }

        current_scope = current_scope->parent;
    }

    e_list_push(arena, &scope->functions, &declaration, sizeof(eASTFunctionDecl));
}

void e_assign(eArena *arena, eASTAssignment assignment, eScope *scope)
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
                    THROW_ERROR(RUNTIME_ERROR, "cannot reassign a constant", 0l);
                }

                eResult result = e_evaluate(arena, assignment.init, scope);

                if(result.value.type != var->value_type)
                {
                    THROW_ERROR(RUNTIME_ERROR, "cannot assign a variable a value of different type", 0l);
                }

                var->value = result.value;

                return;
            }

            current_var = current_var->next;
        }

        current_scope = current_scope->parent;
    }

    THROW_ERROR(RUNTIME_ERROR, "unknown identifier", 0l);
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

    THROW_ERROR(RUNTIME_ERROR, "unknown identifier", 0l);
}
