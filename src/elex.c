#include "elex.h"
#include "eerror.h"
#include <ctype.h>
#include <string.h>

#define ARR_LEN(_arr) (sizeof(_arr) / sizeof(_arr[0]))

#define SINGLE_CHARACTER_TOKEN(_tag, _line, _start) \
    (eToken) { \
        .tag = _tag, \
        .line = _line, \
        .start = _start, \
        .len = 1 \
    }

typedef struct
{
    eString text;
    eTokenTag tag;
} KeywordMap;

static KeywordMap keywords[] = {
    {.text = {.ptr = "var", .len = 3}, .tag = ETK_KEYWORD_VAR},
    {.text = {.ptr = "const", .len = 5}, .tag = ETK_KEYWORD_CONST}
};

static eToken lex_identifier(eString src, size_t start, size_t line)
{
    eToken tk = {0};

    for(size_t i = start; i < src.len; i++)
    {
        char c = src.ptr[i];

        if(!isalpha(c))
        {
            tk.start = start;
            tk.len = i - start;
            tk.line = line;
            tk.tag = ETK_IDENTIFIER;

            for(size_t j = 0; j < ARR_LEN(keywords); j++)
            {
                if(e_string_compare(keywords[j].text, e_string_slice(src, tk.start, keywords[j].text.len)))
                {
                    tk.tag = keywords[j].tag;

                    break;
                }
            }

            break;
        }
    }

    return tk;
}

static eToken lex_number(eString src, size_t start, size_t line)
{
    eToken tk = {0};

    for(size_t i = start; i < src.len; i++)
    {
        char c = src.ptr[i];

        if(!isdigit(c))
        {
            tk.start = start;
            tk.len = i - start;
            tk.line = line;
            tk.tag = ETK_NUMBER;

            break;
        }
    }

    return tk;
}

static eToken lex_string(eString src, size_t start, size_t line)
{
    eToken tk = {0};

    for(size_t i = start + 1; i < src.len; i++)
    {
        char c = src.ptr[i];

        if(c == '"')
        {
            tk.start = start;
            tk.len = i + 1 - start;
            tk.line = line;
            tk.tag = ETK_STRING;

            break;
        }
    }

    return tk;
}

eListNode *e_lex(eArena *arena, eString src)
{
    eListNode *tokens = NULL;

    size_t line = 1;

    size_t i = 0;
    while(i < src.len)
    {
        eToken tk = {0};

        char c = src.ptr[i];

        if(c == '\t' || c == ' ')
        {
            i++;

            continue;
        }
        else if(c == '\n')
        {
            i++;
            line++;
            
            continue;
        }
        else if(c == '"')
        {
            // String literal

            tk = lex_string(src, i, line);
        }
        else if(c == '=')
        {
            tk = SINGLE_CHARACTER_TOKEN(ETK_EQUALS, line, i);
        }
        else if(c == '+')
        {
            tk = SINGLE_CHARACTER_TOKEN(ETK_PLUS, line, i);
        }
        else if(c == '-')
        {
            tk = SINGLE_CHARACTER_TOKEN(ETK_MINUS, line, i);
        }
        else if(c == '*')
        {
            tk = SINGLE_CHARACTER_TOKEN(ETK_ASTERISK, line, i);
        }
        else if(c == '/')
        {
            tk = SINGLE_CHARACTER_TOKEN(ETK_SLASH, line, i);
        }
        else if(c == '%')
        {
            tk = SINGLE_CHARACTER_TOKEN(ETK_PERCENT, line, i);
        }
        else if(c == '(')
        {
            tk = SINGLE_CHARACTER_TOKEN(ETK_L_PAREN, line, i);
        }
        else if(c == ')')
        {
            tk = SINGLE_CHARACTER_TOKEN(ETK_R_PAREN, line, i);
        }
        else if(c == ',')
        {
            tk = SINGLE_CHARACTER_TOKEN(ETK_COMMA, line, i);
        }
        else if(isalpha(c))
        {
            // Identifier
            
            tk = lex_identifier(src, i, line);
        }
        else if(isdigit(c))
        {
            // Number

            tk = lex_number(src, i, line);
        }
        else
        {
            e_errcode = ERR_UNKNOWN_TOKEN;
            e_errline = line;

            tk = (eToken) {
                .tag = ETK_UNKNOWN,
                .line = line,
                .start = i,
                .len = 1
            };
        }

        i += tk.len;
        e_list_push(arena, &tokens, &tk, sizeof(eToken));
    }

    return tokens;
}
