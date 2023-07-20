#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <elex.h>
#include <einterpreter.h>
#include <eerror.h>
#include <eio.h>
#include <estack.h>

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Usage:\n");
        printf("elang <filename>\n");

        return 0;
    }

    eString path = {.ptr = argv[1], .len = strlen(argv[1])};

    eScope scope = e_scope_new(NULL, NULL);

    eFileState file = {
        .is_main = true,
        .path = path
    };

    e_exec_file(path, &scope, &file);

    e_scope_free(&scope);

    return 0;
}

