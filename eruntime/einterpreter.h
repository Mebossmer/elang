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

    bool is_return;
} eResult;

typedef struct
{
    eString identifier;

    eValue value;
    eValueType value_type;

    eAssignmentType type;
} eVariable;

typedef struct
{
    eString identifier;

    eStack args; // eValue
} eFunctionCall;

struct escope
{
    eScope *parent;

    eArena allocator;

    eListNode *variables; // eVariable
    eListNode *functions; // eASTFunctionDecl

    // bool inside_fun; // Wether the scope is inside a function scope
    eASTFunctionDecl *function; // NULL if not inside function
};

/**
 * Returns true on success and false if the file couldn't be read
*/
bool e_exec_file(eString path, eScope *scope);

eScope e_scope_new(/* Nullable */ eScope *parent, /* Nullable */ eASTFunctionDecl *function);

void e_scope_free(eScope *scope);

eResult e_evaluate(eArena *arena, eASTNode *node, eScope *scope);

eResult e_evaluate_body(eArena *arena, eListNode *body, eScope *scope);

eResult e_call(eArena *arena, eFunctionCall call, eScope *scope);

void e_declare(eArena *arena, eString identifier, eValue value, eAssignmentType type, eValueType decl_type, eScope *scope);

void e_declare_function(eArena *arena, eASTFunctionDecl declaration, eScope *scope);

void e_assign(eArena *arena, eASTAssignment assignment, eScope *scope);

eValue e_get_value(eString identifier, eScope *scope);
