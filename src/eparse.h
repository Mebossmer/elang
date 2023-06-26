#pragma once

#include "elex.h"
#include "elist.h"

typedef struct eastnode eASTNode;

typedef enum
{
    AST_EOF,

    AST_NUMERIC_LITERAL,
    AST_STRING_LITERAL,
    AST_BOOL_LITERAL,
    AST_IDENTIFIER,

    AST_ARITHMETIC,
    AST_CONDITION,
    AST_FUNCTION_DECL,
    AST_FUNCTION_CALL,

    AST_DECLARATION,
    AST_ASSIGNMENT,
    AST_IF_STATEMENT,
    AST_WHILE_LOOP
} eASTTag;

typedef enum
{
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    OP_INVALID
} eOperation;

typedef enum
{
    BOP_AND,
    BOP_OR,
    BOP_NOT,
    BOP_IS_EQUAL,
    BOP_IS_LESS,
    BOP_IS_GREATER,

    BOP_INVALID
} eCondition;

typedef enum
{
    AT_CONST,
    AT_VAR,

    AT_INVALID
} eAssignmentType;

typedef struct
{
    eASTNode *rhs;
    eASTNode *lhs;

    eOperation op;
} eASTArithmetic;

typedef struct
{
    eASTNode *rhs;
    eASTNode *lhs;

    eCondition op;
} eASTCondition;

typedef struct
{
    eASTNode *condition;

    eListNode *body; // eASTNode *
} eASTIfStatement;

typedef struct
{
    eASTNode *condition;

    eListNode *body;
} eASTWhileLoop;

typedef struct
{
    int value;
} eASTNumericLiteral;

typedef struct
{
    eString value;
} eASTStringLiteral;

typedef struct
{
    bool value;
} eASTBoolLiteral;

typedef struct
{
    eString identifier;

    eASTNode *init;

    eAssignmentType type;
} eASTDeclaration;

typedef struct
{
    eString identifier;

    eASTNode *init;
} eASTAssignment;

typedef struct
{
    eString identifier;

    eListNode *params; // eString (identifier)
    eListNode *body; // eASTNode *
} eASTFunctionDecl;

typedef struct
{
    eString identifier;

    eListNode *arguments; // eASTNode *
} eASTFunctionCall;

struct eastnode
{
    eASTTag tag;

    union
    {
        eASTNumericLiteral numeric_literal;

        eASTArithmetic arithmetic;

        eASTDeclaration declaration;

        eASTAssignment assignment;

        eASTFunctionCall function_call;

        eASTStringLiteral string_literal;

        eASTBoolLiteral bool_literal;
        
        eString identifier;

        eASTCondition condition;

        eASTIfStatement if_statement;

        eASTWhileLoop while_loop;

        eASTFunctionDecl function_decl;
    };
};

typedef struct
{
    eListNode *tokens;

    size_t index;

    eString src;
} eParser;

eASTNode *e_ast_alloc(eArena *arena, eASTNode node);

eParser e_parser_new(eArena *arena, eListNode *tokens, eString src);

eASTNode *e_parse_factor(eArena *arena, eParser *self);

eASTNode *e_parse_terminal(eArena *arena, eParser *self);

eASTNode *e_parse_expression(eArena *arena, eParser *self);

eASTNode *e_parse_conditional_factor(eArena *arena, eParser *self);

eASTNode *e_parse_condition(eArena *arena, eParser *self);

eASTNode *e_parse_statement(eArena *arena, eParser *self);

eListNode *e_parse_body(eArena *arena, eParser *self);
