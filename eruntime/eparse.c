#include "earena.h"
#include "eparse.h"
#include "eerror.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

eASTNode *e_ast_alloc(eArena *arena, eASTNode node)
{
    eASTNode *new = e_arena_alloc(arena, sizeof(eASTNode));
    *new = node;

    return new;
}

eParser e_parser_new(eListNode *tokens, eString src)
{
    return (eParser) {
        .tokens = tokens,
        .index = 0,
        .src = src
    };
}

static eOperation get_operation(eTokenTag tag)
{
    switch(tag)
    {
    case ETK_PLUS:
        return OP_ADD;

    case ETK_MINUS:
        return OP_SUB;

    case ETK_ASTERISK:
        return OP_MUL;

    case ETK_SLASH:
        return OP_DIV;

    case ETK_PERCENT:
        return OP_MOD;

    default:
        return OP_INVALID;
    }
}

static long to_long(const char *txt, size_t n)
{
    eArena arena = e_arena_new(n + 1);

    char *tmp = e_arena_alloc(&arena, n + 1);
    memcpy(tmp, txt, n);
    tmp[n] = '\0';

    long num = strtol(tmp, NULL, 10);

    e_arena_free(&arena);

    return num;
}

static bool accept(eParser *self, eTokenTag tag)
{
    if(E_LIST_AT(self->tokens, self->index, eToken *)->tag == tag)
    {
        self->index++;

        return true;
    }

    return false;
}

static void expect(eParser *self, eTokenTag tag)
{
    if(accept(self, tag))
    {
        return;
    }

    eToken *tk = E_LIST_AT(self->tokens, self->index, eToken *);
    THROW_ERROR(PARSER_ERROR, "unexpected token", tk->line);
}

eASTNode *e_parse_factor(eArena *arena, eParser *self)
{
    eToken *tk = E_LIST_AT(self->tokens, self->index, eToken *);

    if(accept(self, ETK_NUMBER))
    {
        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_NUMERIC_LITERAL,
            .numeric_literal = (eASTNumericLiteral) {
                .value = to_long(self->src.ptr + tk->start, tk->len)
            }
        });
    }
    else if(accept(self, ETK_STRING))
    {
        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_STRING_LITERAL,
            .string_literal = (eASTStringLiteral) {
                .value = e_string_slice(self->src, tk->start + 1, tk->len - 2)
            }
        });
    }
    else if(accept(self, ETK_IDENTIFIER))
    {
        if(accept(self, ETK_L_PAREN))
        {
            eListNode *arguments = NULL;
            if(!accept(self, ETK_R_PAREN))
            {
                do
                {
                    eASTNode *node = e_parse_expression(arena, self);

                    e_list_push(arena, &arguments, node, sizeof(eASTNode));
                } while(accept(self, ETK_COMMA));

                expect(self, ETK_R_PAREN);
            }

            return e_ast_alloc(arena, (eASTNode) {
                .tag = AST_FUNCTION_CALL,
                .function_call = (eASTFunctionCall) {
                    .identifier = e_string_slice(self->src, tk->start, tk->len),
                    .arguments = arguments
                }
            });
        }

        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_IDENTIFIER,
            .identifier = e_string_slice(self->src, tk->start, tk->len)
        });
    }
    else if(accept(self, ETK_KEYWORD_FALSE))
    {
        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_BOOL_LITERAL,
            .bool_literal = (eASTBoolLiteral) {
                .value = false
            }
        });
    }
    else if(accept(self, ETK_KEYWORD_TRUE))
    {
        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_BOOL_LITERAL,
            .bool_literal = (eASTBoolLiteral) {
                .value = true
            }
        });
    }
    else if(accept(self, ETK_L_PAREN))
    {
        eASTNode *node = e_parse_expression(arena, self);

        expect(self, ETK_R_PAREN);

        return node;
    }
    
    THROW_ERROR(PARSER_ERROR, "syntax error", tk->line);

    return NULL;
}

eASTNode *e_parse_terminal(eArena *arena, eParser *self)
{
    eASTNode *lhs = e_parse_factor(arena, self);
    if(self->index >= e_list_len(self->tokens))
    {
        return lhs;
    }

    while(E_LIST_AT(self->tokens, self->index, eToken *)->tag == ETK_ASTERISK ||
          E_LIST_AT(self->tokens, self->index, eToken *)->tag == ETK_SLASH ||
          E_LIST_AT(self->tokens, self->index, eToken *)->tag == ETK_PERCENT)
    {
        eOperation op = get_operation(E_LIST_AT(self->tokens, self->index, eToken *)->tag);

        self->index++;

        eASTNode *rhs = e_parse_factor(arena, self);

        lhs = e_ast_alloc(arena, (eASTNode) {
            .tag = AST_ARITHMETIC,
            .arithmetic = (eASTArithmetic) {
                .lhs = lhs,
                .rhs = rhs,
                .op = op
            }
        });

        if(self->index >= e_list_len(self->tokens))
        {
            return lhs;
        }
    }

    return lhs;
}

eASTNode *e_parse_expression(eArena *arena, eParser *self)
{
    eASTNode *lhs = e_parse_terminal(arena, self);
    if(self->index >= e_list_len(self->tokens))
    {
        return lhs;
    }

    while(E_LIST_AT(self->tokens, self->index, eToken *)->tag == ETK_PLUS ||
          E_LIST_AT(self->tokens, self->index, eToken *)->tag == ETK_MINUS)
    {
        eOperation op = get_operation(E_LIST_AT(self->tokens, self->index, eToken *)->tag);

        self->index++;

        eASTNode *rhs = e_parse_terminal(arena, self);

        lhs = e_ast_alloc(arena, (eASTNode) {
            .tag = AST_ARITHMETIC,
            .arithmetic = (eASTArithmetic) {
                .lhs = lhs,
                .rhs = rhs,
                .op = op
            }
        });

        if(self->index >= e_list_len(self->tokens))
        {
            return lhs;
        }
    }

    return lhs;
}

static eCondition get_conditional_operator(eTokenTag tag)
{
    switch(tag)
    {
    case ETK_KEYWORD_AND:
        return BOP_AND;
    
    case ETK_KEYWORD_OR:
        return BOP_OR;

    case ETK_DOUBLE_EQUALS:
        return BOP_IS_EQUAL;
    
    case ETK_L_ANGLE:
        return BOP_IS_LESS;

    case ETK_R_ANGLE:
        return BOP_IS_GREATER;

    default:
        return BOP_INVALID;
    }
}

eASTNode *e_parse_conditional_factor(eArena *arena, eParser *self)
{
    if(accept(self, ETK_L_PAREN))
    {
        eASTNode *node = e_parse_condition(arena, self);

        expect(self, ETK_R_PAREN);

        return node;
    }

    eASTNode *lhs = e_parse_expression(arena, self);
    if(self->index >= e_list_len(self->tokens))
    {
        return lhs;
    }

    while(E_LIST_AT(self->tokens, self->index, eToken *)->tag == ETK_L_ANGLE ||
          E_LIST_AT(self->tokens, self->index, eToken *)->tag == ETK_R_ANGLE ||
          E_LIST_AT(self->tokens, self->index, eToken *)->tag == ETK_DOUBLE_EQUALS)
    {
        eCondition op = get_conditional_operator(E_LIST_AT(self->tokens, self->index, eToken *)->tag);

        self->index++;

        eASTNode *rhs = e_parse_expression(arena, self);

        lhs = e_ast_alloc(arena, (eASTNode) {
            .tag = AST_CONDITION,
            .condition = (eASTCondition) {
                .lhs = lhs,
                .rhs = rhs,
                .op = op
            }
        });

        if(self->index >= e_list_len(self->tokens))
        {
            return lhs;
        }
    }

    return lhs;
}

eASTNode *e_parse_condition(eArena *arena, eParser *self)
{
    eASTNode *lhs = e_parse_conditional_factor(arena, self);
    if(self->index >= e_list_len(self->tokens))
    {
        return lhs;
    }

    while(E_LIST_AT(self->tokens, self->index, eToken *)->tag == ETK_KEYWORD_AND ||
          E_LIST_AT(self->tokens, self->index, eToken *)->tag == ETK_KEYWORD_OR)
    {
        eCondition op = get_conditional_operator(E_LIST_AT(self->tokens, self->index, eToken *)->tag);

        self->index++;

        eASTNode *rhs = e_parse_conditional_factor(arena, self);

        lhs = e_ast_alloc(arena, (eASTNode) {
            .tag = AST_CONDITION,
            .condition = (eASTCondition) {
                .lhs = lhs,
                .rhs = rhs,
                .op = op
            }
        });

        if(self->index >= e_list_len(self->tokens))
        {
            return lhs;
        }
    }

    return lhs;
}

static eAssignmentType get_assignment_type(eTokenTag tag, size_t line)
{
    switch(tag)
    {
    case ETK_KEYWORD_VAR:
        return AT_VAR;

    case ETK_KEYWORD_CONST:
        return AT_CONST;

    default:
        THROW_ERROR(PARSER_ERROR, "invalid assignment type", line)
    }
}

static eValueType get_value_type(eTokenTag tag, size_t line)
{
    switch(tag)
    {
    case ETK_KEYWORD_TYPE_STRING:
        return VT_STRING;
    
    case ETK_KEYWORD_TYPE_INT:
        return VT_INT;

    case ETK_KEYWORD_TYPE_BOOL:
        return VT_BOOL;

    case ETK_KEYWORD_TYPE_VOID:
        return VT_VOID;

    default:
        THROW_ERROR(PARSER_ERROR, "invalid type name", line);
    }
}

eASTNode *e_parse_statement(eArena *arena, eParser *self)
{
    if(self->index >= e_list_len(self->tokens))
    {
        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_EOF
        });
    }

    eToken *tk = E_LIST_AT(self->tokens, self->index, eToken *);
    if(accept(self, ETK_KEYWORD_VAR) || accept(self, ETK_KEYWORD_CONST))
    {
        // Variable declaration

        eToken *identifier = E_LIST_AT(self->tokens, self->index, eToken *);

        expect(self, ETK_IDENTIFIER);

        if(accept(self, ETK_DOUBLE_COLON))
        {
            eToken *type_tk = E_LIST_AT(self->tokens, self->index, eToken *);

            self->index++;

            eValueType value_type = get_value_type(type_tk->tag, 0l);
        
            expect(self, ETK_EQUALS);

            return e_ast_alloc(arena, (eASTNode) {
                .tag = AST_DECLARATION,
                .declaration = (eASTDeclaration) {
                    .type = get_assignment_type(tk->tag, 0l),
                    .init = e_parse_expression(arena, self),
                    .identifier = e_string_slice(self->src, identifier->start, identifier->len),
                    .value_type = value_type
                }
            });
        }

        expect(self, ETK_EQUALS);

        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_DECLARATION,
            .declaration = (eASTDeclaration) {
                .type = get_assignment_type(tk->tag, 0l),
                .init = e_parse_expression(arena, self),
                .identifier = e_string_slice(self->src, identifier->start, identifier->len),
                .value_type = VT_VOID
            }
        });
    }
    else if(accept(self, ETK_IDENTIFIER))
    {
        if(accept(self, ETK_L_PAREN))
        {
            eListNode *arguments = NULL;
            if(!accept(self, ETK_R_PAREN))
            {
                do
                {
                    eASTNode *node = e_parse_expression(arena, self);

                    e_list_push(arena, &arguments, node, sizeof(eASTNode));
                } while(accept(self, ETK_COMMA));

                expect(self, ETK_R_PAREN);
            }

            return e_ast_alloc(arena, (eASTNode) {
                .tag = AST_FUNCTION_CALL,
                .function_call = (eASTFunctionCall) {
                    .identifier = e_string_slice(self->src, tk->start, tk->len),
                    .arguments = arguments
                }
            });
        }
        else if(accept(self, ETK_EQUALS))
        {
            return e_ast_alloc(arena, (eASTNode) {
                .tag = AST_ASSIGNMENT,
                .assignment = (eASTAssignment) {
                    .init = e_parse_expression(arena, self),
                    .identifier = e_string_slice(self->src, tk->start, tk->len)
                }
            });
        }
    }
    else if(accept(self, ETK_KEYWORD_IF))
    {
        eASTNode *condition = e_parse_condition(arena, self);
        if(condition == NULL)
        {
            return NULL;
        }

        eListNode *body = e_parse_body(arena, self);

        eListNode *else_body = NULL;
        if(accept(self, ETK_KEYWORD_ELSE))
        {
            else_body = e_parse_body(arena, self);
        }

        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_IF_STATEMENT,
            .if_statement = (eASTIfStatement) {
                .condition = condition,
                .body = body,
                .else_body = else_body
            }
        });
    }
    else if(accept(self, ETK_KEYWORD_WHILE))
    {
        // This is basically the same as the if statement

        eASTNode *condition = e_parse_condition(arena, self);
        if(condition == NULL)
        {
            return NULL;
        }

        eListNode *body = e_parse_body(arena, self);
        if(body == NULL)
        {
            return NULL;
        }

        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_WHILE_LOOP,
            .while_loop = (eASTWhileLoop) {
                .condition = condition,
                .body = body
            }
        });
    }
    else if(accept(self, ETK_KEYWORD_FUN))
    {
        bool is_extern = false;
        if(accept(self, ETK_KEYWORD_EXTERN))
        {
            is_extern = true;
        }
        
        eToken *id = E_LIST_AT(self->tokens, self->index, eToken *);

        expect(self, ETK_IDENTIFIER);
        expect(self, ETK_L_PAREN);

        eListNode *params = NULL;

        if(E_LIST_AT(self->tokens, self->index, eToken *)->tag == ETK_IDENTIFIER)
        {
            do
            {
                eToken *param_tk = E_LIST_AT(self->tokens, self->index, eToken *);

                self->index++;

                eString param_ident = e_string_slice(self->src, param_tk->start, param_tk->len);

                expect(self, ETK_DOUBLE_COLON);

                eToken *param_type = E_LIST_AT(self->tokens, self->index, eToken *);
                eValueType value_type = get_value_type(param_type->tag, 0l);

                self->index++;

                e_list_push(arena, &params, &(eASTFunctionParam) {
                    .identifier = param_ident,
                    .value_type = value_type
                }, sizeof(eASTFunctionParam));
            } while(accept(self, ETK_COMMA));
        }

        expect(self, ETK_R_PAREN);

        eValueType return_type = VT_VOID;
        if(accept(self, ETK_DOUBLE_COLON))
        {
            eToken *return_type_tk = E_LIST_AT(self->tokens, self->index, eToken *);
            self->index++;

            return_type = get_value_type(return_type_tk->tag, 0l);
        }

        eListNode *body = e_parse_body(arena, self);
        if(!body)
        {
            return NULL;
        }

        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_FUNCTION_DECL,
            .function_decl = (eASTFunctionDecl) {
                .identifier = e_string_slice(self->src, id->start, id->len),
                .return_type = return_type,
                .params = params,
                .body = body,
                .is_extern = true
            }
        });
    }
    else if(accept(self, ETK_KEYWORD_RETURN))
    {
        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_RETURN,
            .return_stmt = (eASTReturn) {
                .arg = e_parse_expression(arena, self)
            }
        });
    }
    else if(accept(self, ETK_KEYWORD_IMPORT))
    {
        eToken *path_tk = E_LIST_AT(self->tokens, self->index, eToken *);
        eString path = e_string_slice(self->src, path_tk->start, path_tk->len);

        expect(self, ETK_STRING);

        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_IMPORT,
            .import_stmt = (eASTImport) {
                .path = path
            }
        });
    }
    else if(accept(self, ETK_EOF))
    {
        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_EOF
        });
    }

    THROW_ERROR(PARSER_ERROR, "unknown statement", tk->line);

    return NULL;
}

eListNode *e_parse_body(eArena *arena, eParser *self)
{
    expect(self, ETK_L_CURLY_BRACE);

    eListNode *stmts = NULL;

    if(accept(self, ETK_R_CURLY_BRACE))
    {
        return NULL;
    }

    do
    {
        eASTNode *stmt = e_parse_statement(arena, self);
        if(stmt == NULL)
        {
            return NULL;
        }

        if(stmt->tag == AST_EOF)
        {
            return NULL;
        }

        e_list_push(arena, &stmts, stmt, sizeof(eASTNode));
    } while(!accept(self, ETK_R_CURLY_BRACE));

    return stmts;
}
