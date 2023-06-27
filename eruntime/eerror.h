#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define LEXER_ERROR "Lexer error"
#define PARSER_ERROR "Parser error"
#define RUNTIME_ERROR "Runtime error"

#define THROW_ERROR(_type, _msg, _line) \
    { \
    fprintf(stderr, _type " on line %ld: " _msg "\n", _line); \
    exit(-1); \
    }
