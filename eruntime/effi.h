#pragma once

#include "einterpreter.h"

typedef eResult(* eFunctionPtr)(eArena *arena, eScope *scope, eListNode *arguments);

eResult e_ffi_call(eString name, eString lib, eArena *arena, eScope *scope, eListNode *arguments);
