#pragma once

#include "einterpreter.h"

eResult __e_print(eArena *arena, eScope *scope, eListNode *arguments);

eResult __e_exit(eArena *arena, eScope *scope, eListNode *arguments);
