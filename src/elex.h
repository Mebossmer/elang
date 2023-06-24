#pragma once

#include "elist.h"
#include <stdint.h>
#include <stddef.h>

typedef struct etokenlist eTokenList;

typedef enum
{
    ETK_UNKNOWN,

    ETK_IDENTIFIER,
    ETK_NUMBER,
    ETK_STRING,

    ETK_KEYWORD_VAR,
    ETK_KEYWORD_CONST,

    ETK_EQUALS,
    ETK_PLUS,
    ETK_MINUS,
    ETK_ASTERISK,
    ETK_SLASH,
    ETK_PERCENT,

    ETK_L_PAREN,
    ETK_R_PAREN,
    ETK_COMMA
} eTokenTag;

typedef struct
{
    eTokenTag tag;
    size_t line, start, len;
} eToken;

eListNode *e_lex(const char *src);
