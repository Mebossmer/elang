#include "eerror.h"

eError e_errcode = ERR_NONE;
size_t e_errline = 0;

const char *e_get_error()
{
    switch(e_errcode)
    {
    case ERR_NONE:
        return "no error";

    case ERR_UNKNOWN_TOKEN:
        return "unknown token";

    case ERR_UNEXPECTED_TOKEN:
        return "unexpected token";

    case ERR_SYNTAX:
        return "syntax error";

    case ERR_UNKNOWN_STATEMENT:
        return "unknown statement";

    case ERR_ARGUMENT_COUNT:
        return "wrong amount of arguments provided";

    default:
        return "unknown error";
    }
}

size_t e_get_error_line()
{
    return e_errline;
}
