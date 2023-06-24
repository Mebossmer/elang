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

/*
void e_ast_free(eASTNode *node)
{
    switch(node->tag)
    {
    case AST_ARITHMETIC:
        e_ast_free(node->arithmetic.lhs);
        e_ast_free(node->arithmetic.rhs);

        break;

    case AST_DECLARATION:
        free(node->declaration.identifier);
        e_ast_free(node->declaration.init);

        break;

    case AST_ASSIGNMENT:
        free(node->assignment.identifier);
        e_ast_free(node->assignment.init);

        break;

    case AST_FUNCTION_CALL:
        free(node->function_call.identifier);
        eListNode *current = node->function_call.arguments;
        while(current != NULL)
        {
            e_ast_free(current->data);

            current = current->next;
        }

        break;

    case AST_STRING_LITERAL:
        free(node->string_literal.value);

        break;

    default:
        break;
    }

    free(node);
}
*/

eParser e_parser_new(eArena *arena, eListNode *tokens, eString src)
{
    /*
    size_t len = strlen(src);
    char *tmp = e_arena_alloc(arena, len + 1);
    memcpy(tmp, src, len);
    tmp[len] = '\0';
    */

    return (eParser) {
        .tokens = tokens,
        .index = 0,
        .src = src
    };
}

/*
void e_parser_free(eParser *self)
{
    free(self->src);
}
*/

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
    char *tmp = malloc(n + 1);
    memcpy(tmp, txt, n);
    tmp[n] = '\0';

    long num = strtol(tmp, NULL, 10);

    free(tmp);

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

static bool expect(eParser *self, eTokenTag tag)
{
    if(accept(self, tag))
    {
        return true;
    }

    e_errcode = ERR_UNEXPECTED_TOKEN;
    e_errline = E_LIST_AT(self->tokens, self->index, eToken *)->line;
    // fprintf(stderr, "Unexpected token at index %ld\n", self->index);

    return false;
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
        char *tmp = e_arena_alloc(arena, tk->len - 1);
        memcpy(tmp, self->src.ptr + tk->start + 1, tk->len - 2);
        tmp[tk->len - 2] = '\0';

        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_STRING_LITERAL,
            .string_literal = (eASTStringLiteral) {
                .value = tmp
            }
        });
    }
    else if(accept(self, ETK_IDENTIFIER))
    {
        char *tmp = e_arena_alloc(arena, tk->len + 1);
        memcpy(tmp, self->src.ptr + tk->start, tk->len);
        tmp[tk->len] = '\0';

        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_IDENTIFIER,
            .identifier = tmp
        });
    }
    else if(accept(self, ETK_L_PAREN))
    {
        eASTNode *node = e_parse_expression(arena, self);

        if(!expect(self, ETK_R_PAREN))
        {
            return NULL;
        }

        return node;
    }
    
    e_errcode = ERR_SYNTAX;
    e_errline = tk->line;

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

static eAssignmentType get_assignment_type(eTokenTag tag)
{
    switch(tag)
    {
    case ETK_KEYWORD_VAR:
        return AT_VAR;

    case ETK_KEYWORD_CONST:
        return AT_CONST;

    default:
        return AT_INVALID;
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

        if(!expect(self, ETK_IDENTIFIER))
        {
            return NULL;
        }
        if(!expect(self, ETK_EQUALS))
        {
            return NULL;
        }

        char *id = e_arena_alloc(arena, identifier->len + 1);
        memcpy(id, self->src.ptr + identifier->start, identifier->len);
        id[identifier->len] = '\0';

        return e_ast_alloc(arena, (eASTNode) {
            .tag = AST_DECLARATION,
            .declaration = (eASTDeclaration) {
                .type = get_assignment_type(tk->tag),
                .init = e_parse_expression(arena, self),
                .identifier = id
            }
        });
    }
    else if(accept(self, ETK_IDENTIFIER))
    {
        // Variable assignment
        if(accept(self, ETK_L_PAREN))
        {
            char *id = e_arena_alloc(arena, tk->len + 1);
            memcpy(id, self->src.ptr + tk->start, tk->len);
            id[tk->len] = '\0';

            eListNode *arguments = NULL;
            do
            {
                eASTNode *node = e_parse_expression(arena, self);

                e_list_push(arena, &arguments, node, sizeof(eASTNode));
            } while(accept(self, ETK_COMMA));

            if(!expect(self, ETK_R_PAREN))
            {
                return NULL;
            }

            return e_ast_alloc(arena, (eASTNode) {
                .tag = AST_FUNCTION_CALL,
                .function_call = (eASTFunctionCall) {
                    .identifier = id,
                    .arguments = arguments
                }
            });
        }
        else if(accept(self, ETK_EQUALS))
        {
            char *id = e_arena_alloc(arena, tk->len + 1);
            memcpy(id, self->src.ptr + tk->start, tk->len);
            id[tk->len] = '\0';

            return e_ast_alloc(arena, (eASTNode) {
                .tag = AST_ASSIGNMENT,
                .assignment = (eASTAssignment) {
                    .init = e_parse_expression(arena, self),
                    .identifier = id
                }
            });
        }
    }

    e_errline = tk->line;
    e_errcode = ERR_UNKNOWN_STATEMENT;

    return NULL;
}
