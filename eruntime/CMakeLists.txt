project("eruntime" C)

add_library("eruntime" STATIC
    "earena.c"
    "ebuiltin.c"
    "einterpreter.c"
    "elex.c"
    "elist.c"
    "eparse.c"
    "estring.c"
    "eio.c"
)
