#pragma once

#include "einterpreter.h"

typedef eResult(* eFunctionPtr)(eArena *arena, eScope *scope, eStack *arguments);

typedef struct
{
    eString name;

    eFunctionPtr ptr;

    size_t num_args;
} eFunctionDef;

eResult e_ffi_call(eString name, eString lib, eArena *arena, eScope *scope, eStack *arguments);
