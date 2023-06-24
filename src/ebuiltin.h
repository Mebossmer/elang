#pragma once

#include "einterpreter.h"

eValue __e_print(eArena *arena, eScope *scope, eListNode *arguments);

eValue __e_exit(eArena *arena, eScope *scope, eListNode *arguments);
