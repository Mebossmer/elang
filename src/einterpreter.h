#pragma once

#include "elist.h"
#include "eparse.h"

typedef enum
{
    VT_INT,
    VT_STRING,
    VT_BOOL,

    VT_INVALID,
    VT_ERROR
} eValueType;

typedef struct
{
    eValueType type;

    union
    {
        int integer;

        eString string;

        bool boolean;
    };
} eValue;

typedef struct
{
    eString identifier;

    eValue value;

    eAssignmentType type;
} eVariable;

typedef struct
{
    eListNode *variables;
} eScope;

eValue e_evaluate(eArena *arena, eASTNode *node, eScope *scope);

void e_evaluate_body(eArena *arena, eListNode *body, eScope *scope);

eValue e_call(eArena *arena, eASTFunctionCall call, eScope *scope);

void e_declare(eArena *arena, eASTDeclaration declaration, eScope *scope);

void e_assign(eArena *arena, eASTAssignment assignment, eScope *scope);

eValue e_get_value(eString identifier, eScope *scope);
