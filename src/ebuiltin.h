#pragma once

#include "einterpreter.h"

eValue __e_print(eScope *scope, eListNode *arguments);

eValue __e_exit(eScope *scope, eListNode *arguments);
