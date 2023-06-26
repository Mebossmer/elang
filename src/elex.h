#pragma once

#include "elist.h"
#include "estring.h"
#include <stdint.h>
#include <stddef.h>

typedef struct etokenlist eTokenList;

typedef enum
{
    ETK_EOF,
    ETK_UNKNOWN,

    ETK_IDENTIFIER,
    ETK_NUMBER,
    ETK_STRING,

    ETK_KEYWORD_VAR,
    ETK_KEYWORD_CONST,
    ETK_KEYWORD_TRUE,
    ETK_KEYWORD_FALSE,
    ETK_KEYWORD_IF,
    ETK_KEYWORD_WHILE,
    ETK_KEYWORD_AND,
    ETK_KEYWORD_OR,
    ETK_KEYWORD_FUN,

    ETK_EQUALS,
    ETK_DOUBLE_EQUALS,
    ETK_PLUS,
    ETK_MINUS,
    ETK_ASTERISK,
    ETK_SLASH,
    ETK_PERCENT,
    ETK_AMPERSAND,
    ETK_PIPE,
    ETK_EXCLAMATION,
    ETK_L_ANGLE,
    ETK_R_ANGLE,
    ETK_L_CURLY_BRACE,
    ETK_R_CURLY_BRACE,

    ETK_L_PAREN,
    ETK_R_PAREN,
    ETK_COMMA
} eTokenTag;

typedef struct
{
    eTokenTag tag;
    size_t line, start, len;
} eToken;

eListNode *e_lex(eArena *arena, eString src);
