#pragma once

#include <stddef.h>

typedef enum
{
    ERR_NONE,

    ERR_UNKNOWN_TOKEN,
    ERR_UNEXPECTED_TOKEN,
    ERR_SYNTAX,
    ERR_UNKNOWN_STATEMENT,
    ERR_ARGUMENT_COUNT,
    ERR_INVALID_BOOL_OPERATOR,
    ERR_UNKNOWN_IDENTIFIER,
    ERR_NAME_CONFLICT,
    ERR_CONST_REASSIGNMENT,
    ERR_MISSING_QUOTATION_MARK
} eError;

extern eError e_errcode;
extern size_t e_errline;

const char *e_get_error();

size_t e_get_error_line();
