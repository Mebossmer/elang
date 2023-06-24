#pragma once

#include "elist.h"
#include "eparse.h"

typedef enum
{
    VT_INT,
    VT_STRING,

    VT_INVALID,
    VT_ERROR
} eValueType;

typedef struct
{
    eValueType type;

    union
    {
        int integer;

        char *string;
    };
} eValue;

typedef struct
{
    char *identifier;

    eValue value;

    eAssignmentType type;
} eVariable;

typedef struct
{
    eListNode *variables;
} eScope;

eValue e_evaluate(eASTNode *node, eScope *scope);

eValue e_call(eASTFunctionCall call, eScope *scope);

void e_declare(eASTDeclaration declaration, eScope *scope);

void e_assign(eASTAssignment assignment, eScope *scope);

eValue e_get_value(const char *identifier, eScope *scope);

void e_free_scope(eScope *self);

void e_free_variable(eVariable *self);
