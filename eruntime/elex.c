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

typedef struct
{
    char c;
    eTokenTag tag;
} SpecialMap;

static KeywordMap keywords[] = {
    {.text = {.ptr = "var", .len = 3}, .tag = ETK_KEYWORD_VAR},
    {.text = {.ptr = "const", .len = 5}, .tag = ETK_KEYWORD_CONST},
    {.text = {.ptr = "true", .len = 4}, .tag = ETK_KEYWORD_TRUE},
    {.text = {.ptr = "false", .len = 5}, .tag = ETK_KEYWORD_FALSE},
    {.text = {.ptr = "if", .len = 2}, .tag = ETK_KEYWORD_IF},
    {.text = {.ptr = "else", .len = 4}, .tag = ETK_KEYWORD_ELSE},
    {.text = {.ptr = "while", .len = 5}, .tag = ETK_KEYWORD_WHILE},
    {.text = {.ptr = "and", .len = 3}, .tag = ETK_KEYWORD_AND},
    {.text = {.ptr = "or", .len = 2}, .tag = ETK_KEYWORD_OR},
    {.text = {.ptr = "fun", .len = 3}, .tag = ETK_KEYWORD_FUN},
    {.text = {.ptr = "string", .len = 6}, .tag = ETK_KEYWORD_TYPE_STRING},
    {.text = {.ptr = "int", .len = 3}, .tag = ETK_KEYWORD_TYPE_INT},
    {.text = {.ptr = "bool", .len = 4}, .tag = ETK_KEYWORD_TYPE_BOOL},
    {.text = {.ptr = "void", .len = 4}, .tag = ETK_KEYWORD_TYPE_VOID},
    {.text = {.ptr = "return", .len = 6}, .tag = ETK_KEYWORD_RETURN}
};

static SpecialMap special[] = {
    // {.c = '=', .tag = ETK_EQUALS},
    {.c = '+', .tag = ETK_PLUS},
    {.c = '-', .tag = ETK_MINUS},
    {.c = '*', .tag = ETK_ASTERISK},
    {.c = '/', .tag = ETK_SLASH},
    {.c = '%', .tag = ETK_PERCENT},
    {.c = '(', .tag = ETK_L_PAREN},
    {.c = ')', .tag = ETK_R_PAREN},
    {.c = ',', .tag = ETK_COMMA},
    {.c = '<', .tag = ETK_L_ANGLE},
    {.c = '>', .tag = ETK_R_ANGLE},
    {.c = '&', .tag = ETK_AMPERSAND},
    {.c = '|', .tag = ETK_PIPE},
    {.c = '{', .tag = ETK_L_CURLY_BRACE},
    {.c = '}', .tag = ETK_R_CURLY_BRACE},
    {.c = '!', .tag = ETK_EXCLAMATION},
    {.c = ':', .tag = ETK_DOUBLE_COLON}
};

static eToken lex_equals_sign(eString src, size_t start, size_t line)
{
    eToken tk = {0};

    char c = src.ptr[start + 1];

    tk.start = start;
    tk.len = 1;
    tk.line = line;
    tk.tag = ETK_EQUALS;

    if(c == '=')
    {
        tk.len = 2;
        tk.tag = ETK_DOUBLE_EQUALS;
    }

    return tk;
}

static eToken lex_special_character(eString src, size_t start, size_t line)
{
    eToken tk = {0};

    char c = src.ptr[start];

    for(size_t i = 0; i < ARR_LEN(special); i++)
    {
        if(c == special[i].c)
        {
            tk.start = start;
            tk.len = 1;
            tk.line = line;
            tk.tag = special[i].tag;

            goto done;
        }
    }

    THROW_ERROR(LEXER_ERROR, "unknown token", line);

    tk.start = start;
    tk.len = 1;
    tk.line = line;
    tk.tag = ETK_UNKNOWN;

done:
    return tk;
}

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
        else if(c == '\0' || c == '\n')
        {
            THROW_ERROR(LEXER_ERROR, "missing quotation mark", line);

            return (eToken) {
                .tag = ETK_UNKNOWN,
                .start = start,
                .line = line,
                .len = 1
            };
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

        if(c == '\0')
        {
            e_list_push(arena, &tokens, &(eToken) {
                .tag = ETK_EOF,
                .len = 0,
                .line = line,
                .start = i
            }, sizeof(eToken));

            return tokens;
        }
        else if(c == '\t' || c == ' ')
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
            tk = lex_string(src, i, line);
        }
        else if(c == '=')
        {
            tk = lex_equals_sign(src, i, line);
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
            tk = lex_special_character(src, i, line);
        }

        i += tk.len;
        e_list_push(arena, &tokens, &tk, sizeof(eToken));

        if(tk.tag == ETK_UNKNOWN)
        {
            return tokens;
        }
    }

    return tokens;
}
