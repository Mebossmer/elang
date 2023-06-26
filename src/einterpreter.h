#pragma once

#include "elist.h"
#include "eparse.h"

typedef struct escope eScope;

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

struct escope
{
    eScope *parent;

    eArena allocator;

    eListNode *variables; // eVariable
    eListNode *functions; // eASTFunctionDecl
};

eScope e_scope_new(/* Nullable */ eScope *parent);

void e_scope_free(eScope *scope);

eValue e_evaluate(eArena *arena, eASTNode *node, eScope *scope);

void e_evaluate_body(eArena *arena, eListNode *body, eScope *scope);

eValue e_call(eArena *arena, eASTFunctionCall call, eScope *scope);

bool e_declare(eArena *arena, eASTDeclaration declaration, eScope *scope);

bool e_declare_function(eArena *arena, eASTFunctionDecl declaration, eScope *scope);

bool e_assign(eArena *arena, eASTAssignment assignment, eScope *scope);

eValue e_get_value(eString identifier, eScope *scope);
