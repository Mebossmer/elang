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

    case ERR_INVALID_BOOL_OPERATOR:
        return "invalid bool operator";

    case ERR_UNKNOWN_IDENTIFIER:
        return "unknown identifier";

    case ERR_NAME_CONFLICT:
        return "name conflict";

    case ERR_CONST_REASSIGNMENT:
        return "cannot reassign a constant variable";

    case ERR_MISSING_QUOTATION_MARK:
        return "missing quotation mark";

    default:
        return "unknown error";
    }
}

size_t e_get_error_line()
{
    return e_errline;
}
