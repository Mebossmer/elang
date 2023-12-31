#pragma once

#include "elex.h"
#include "elist.h"
#include "estack.h"

typedef struct eastnode eASTNode;

typedef enum
{
    AST_EOF,

    AST_NUMERIC_LITERAL,
    AST_STRING_LITERAL,
    AST_BOOL_LITERAL,
    AST_IDENTIFIER,
    AST_MEMBER,

    AST_ARITHMETIC,
    AST_CONDITION,
    AST_FUNCTION_DECL,
    AST_FUNCTION_CALL,
    AST_RETURN,
    AST_IMPORT,

    AST_DECLARATION,
    AST_ASSIGNMENT,
    AST_IF_STATEMENT,
    AST_WHILE_LOOP
} eASTTag;

typedef enum
{
    VT_INT,
    VT_STRING,
    VT_BOOL,

    VT_VOID // Not defined
} eValueType;

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
    AT_VAR
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
    eListNode *else_body; // eASTNode * (NULL if no body)
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
    eValueType value_type;

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
    eValueType return_type;

    eListNode *params; // eFunctionParam
    eListNode *body; // eASTNode *

    bool is_extern;
} eASTFunctionDecl;

typedef struct
{
    eASTNode *base; // identifier or member

    eListNode *arguments; // eASTNode *
} eASTFunctionCall;

typedef struct
{
    eString identifier;
    eValueType value_type;
} eASTFunctionParam;

typedef struct
{
    eASTNode *arg;
} eASTReturn;

typedef struct
{
    eString path;

    eString identifier;
} eASTImport;

typedef struct
{
    eString identifier;

    eASTNode *base; // identifier or member
} eASTMember;

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

        eASTReturn return_stmt;

        eASTImport import_stmt;

        eASTMember member;
    };
};

typedef struct
{
    eListNode *tokens;

    size_t index;

    eString src;
} eParser;

eASTNode *e_ast_alloc(eArena *arena, eASTNode node);

eParser e_parser_new(eListNode *tokens, eString src);

eASTNode *e_parse_member(eArena *arena, eParser *self);

eASTNode *e_parse_factor(eArena *arena, eParser *self);

eASTNode *e_parse_terminal(eArena *arena, eParser *self);

eASTNode *e_parse_expression(eArena *arena, eParser *self);

eASTNode *e_parse_conditional_factor(eArena *arena, eParser *self);

eASTNode *e_parse_condition(eArena *arena, eParser *self);

eASTNode *e_parse_statement(eArena *arena, eParser *self);

eListNode *e_parse_body(eArena *arena, eParser *self);
