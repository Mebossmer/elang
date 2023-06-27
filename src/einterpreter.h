#pragma once

#include "elist.h"
#include "eparse.h"

typedef struct escope eScope;

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
    eValue value;

    bool is_void;
} eResult;

typedef struct
{
    eString identifier;

    eValue value;
    eValueType value_type;

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

eResult e_evaluate(eArena *arena, eASTNode *node, eScope *scope);

eResult e_evaluate_body(eArena *arena, eListNode *body, eScope *scope);

eResult e_call(eArena *arena, eASTFunctionCall call, eScope *scope);

void e_declare(eArena *arena, eASTDeclaration declaration, eScope *scope);

void e_declare_function(eArena *arena, eASTFunctionDecl declaration, eScope *scope);

void e_assign(eArena *arena, eASTAssignment assignment, eScope *scope);

eValue e_get_value(eString identifier, eScope *scope);
