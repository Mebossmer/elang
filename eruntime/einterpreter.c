#include "einterpreter.h"
#include "eerror.h"
#include "eio.h"
#include "effi.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

bool e_exec_file(eString path, eScope *scope, eFileState *file)
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
        eResult value = e_evaluate(&scope->allocator, expr, scope, file);

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

eResult e_evaluate(eArena *arena, eASTNode *node, eScope *scope, eFileState *file)
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
        eValue value = e_get_value(node->identifier, scope, file);

        return (eResult) {
            .value = value,
            .is_void = false,
            .is_return = false
        };
    }

    case AST_ARITHMETIC: {
        eResult lhs = e_evaluate(arena, node->arithmetic.lhs, scope, file);
        eResult rhs = e_evaluate(arena, node->arithmetic.rhs, scope, file);

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
        eResult result = e_evaluate(arena, node->declaration.init, scope, file);
        if(result.is_void)
        {
            THROW_ERROR(RUNTIME_ERROR, "cannot assign void to a variable", 0l);
        }

        e_declare(arena, node->declaration.identifier, result.value, node->declaration.type, node->declaration.value_type, scope, file);

        return (eResult) {.value = {0}, .is_void = true, .is_return = false};
    }

    case AST_ASSIGNMENT: {
        e_assign(arena, node->assignment, scope, file);

        return (eResult) {.value = {0}, .is_void = true, .is_return = false};
    }

    case AST_FUNCTION_CALL: {
        eStack args = e_stack_new(arena, 128, sizeof(eValue));

        eListNode *current = node->function_call.arguments;
        while(current != NULL)
        {
            eResult result = e_evaluate(arena, current->data, scope, file);
            if(result.is_void)
            {
                THROW_ERROR(RUNTIME_ERROR, "cannot accept void as argument", 0l);
            }

            e_stack_push(arena, &args, &result.value);

            current = current->next;
        }

        return e_call(arena, (eFunctionCall) {
            .args = args,
            .identifier = node->function_call.base->identifier
        }, scope, file);
    }

    case AST_IF_STATEMENT: {
        eResult condition = e_evaluate(arena, node->if_statement.condition, scope, file);

        eResult result = {0};
        if(condition.value.boolean)
        {
            result = e_evaluate_body(arena, node->if_statement.body, scope, file);
        }
        else if(node->if_statement.else_body != NULL)
        {
            result = e_evaluate_body(arena, node->if_statement.else_body, scope, file);
        }

        // return (eResult) {.value = {0}, .is_void = true, .is_return = false};
        return result;
    }

    case AST_WHILE_LOOP: {
        eResult condition = e_evaluate(arena, node->while_loop.condition, scope, file);

        while(condition.value.boolean)
        {
            e_evaluate_body(arena, node->while_loop.body, scope, file);

            condition = e_evaluate(arena, node->while_loop.condition, scope, file);
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
        eResult lhs = e_evaluate(arena, node->condition.lhs, scope, file);
        eResult rhs = e_evaluate(arena, node->condition.rhs, scope, file);

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
        e_declare_function(arena, node->function_decl, scope, file);

        return (eResult) {.value = {0}, .is_void = true, .is_return = false};

    case AST_RETURN: {
        if(scope->function == NULL)
        {
            THROW_ERROR(RUNTIME_ERROR, "cannot return ouside of function", 0l);
        }

        eResult return_value = e_evaluate(arena, node->return_stmt.arg, scope, file);
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
        eString path = e_string_slice(node->import_stmt.path, 1, node->import_stmt.path.len - 2);

        eFileState imported = {
            .identifier = path,
            .is_main = false
        };

        e_exec_file(path, scope, file);

        return (eResult) {.value = {0}, .is_void = true, .is_return = false};
    }

    default: {
        THROW_ERROR(RUNTIME_ERROR, "unknown expression", 0l);
    }
    }
}

eResult e_evaluate_body(eArena *arena, eListNode *body, eScope *scope, eFileState *file)
{
    eListNode *current = body;
    while(current != NULL)
    {
        eResult result = e_evaluate(arena, (eASTNode *) current->data, scope, file);
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

eResult e_call(eArena *arena, eFunctionCall call, eScope *scope, eFileState *file)
{
    eASTFunctionDecl *function = get_function(call.identifier, scope);
    if(function != NULL)
    {
        if(e_stack_len(&call.args) != e_list_len(function->params))
        {
            THROW_ERROR(RUNTIME_ERROR, "wrong amount of args provided", 0l);
        }

        // Declare all arguments as variables
        eScope fn_scope = e_scope_new(scope, function);
        eListNode *current_param = function->params;
        for(size_t i = 0; i < e_stack_len(&call.args); i++)
        {
            eASTFunctionParam *param = (eASTFunctionParam *) current_param->data;
            eValue value = E_STACK_POP(&call.args, eValue);

            e_declare(&fn_scope.allocator, param->identifier, value, AT_VAR, param->value_type, &fn_scope, file);

            current_param = current_param->next;
        }

        // Execute the function
        eListNode *current = function->body;
        while(current != NULL)
        {
            eASTNode *node = (eASTNode *) current->data;

            eResult result = e_evaluate(arena, node, &fn_scope, file);
            if(result.is_return)
            {
                // Return from function
                e_scope_free(&fn_scope);

                return result;
            }
            
            current = current->next;
        }

        e_scope_free(&fn_scope);

        if(function->return_type != VT_VOID)
        {
            THROW_ERROR(RUNTIME_ERROR, "no return statement found inside function", 0l);
        }

        return (eResult) {.value = {0}, .is_void = true, .is_return = false};
    }

    // TODO: this is temporary
    return e_ffi_call(call.identifier, (eString) {.ptr = "./build/elibrary/libelibrary.so", .len = 31}, arena, scope, &call.args);
}

void e_declare(eArena *arena, eString identifier, eValue value, eAssignmentType type, eValueType decl_type, eScope *scope, eFileState *file)
{
    // Search for variables with the same name
    eScope *current_scope = scope;
    while(current_scope != NULL)
    {
        eListNode *current = current_scope->variables;
        while(current != NULL)
        {
            // Throw an error if there is already a variable with that name
            eVariable *var = (eVariable *) current->data;
            if(e_string_compare(var->identifier, identifier))
            {
                THROW_ERROR(RUNTIME_ERROR, "name conflict", 0l);
            }
        
            current = current->next;
        }

        current_scope = current_scope->parent;
    }

    if(value.type != decl_type && decl_type != VT_VOID)
    {
        THROW_ERROR(RUNTIME_ERROR, "type conflict", 0l);
    }

    // Push the variable to the list
    e_list_push(arena, &scope->variables, &(eVariable) {
        .identifier = identifier,
        .value = value,
        .type = type,
        .value_type = value.type
    }, sizeof(eVariable));
}

void e_declare_function(eArena *arena, eASTFunctionDecl declaration, eScope *scope, eFileState *file)
{
    // Search for function with the same name
    eScope *current_scope = scope;
    while(current_scope != NULL)
    {
        eListNode *current = scope->functions;
        while(current != NULL)
        {
            // Throw an error if a function with that name already exists
            eASTFunctionDecl *decl = (eASTFunctionDecl *) current->data;
            if(e_string_compare(decl->identifier, declaration.identifier))
            {
                THROW_ERROR(RUNTIME_ERROR, "name conflict", 0l);
            }

            current = current->next;
        }

        current_scope = current_scope->parent;
    }

    // Push the function to the list
    e_list_push(arena, &scope->functions, &declaration, sizeof(eASTFunctionDecl));
}

void e_assign(eArena *arena, eASTAssignment assignment, eScope *scope, eFileState *file)
{
    // Search for the corresponding variable
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

                eResult result = e_evaluate(arena, assignment.init, scope, file);

                if(result.value.type != var->value_type)
                {
                    THROW_ERROR(RUNTIME_ERROR, "cannot assign a variable a value of different type", 0l);
                }

                // Assign
                var->value = result.value;

                return;
            }

            current_var = current_var->next;
        }

        current_scope = current_scope->parent;
    }

    THROW_ERROR(RUNTIME_ERROR, "unknown identifier", 0l);
}

eValue e_get_value(eString identifier, eScope *scope, eFileState *file)
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
