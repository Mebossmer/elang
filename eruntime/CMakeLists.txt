project("eruntime" C)

add_library("eruntime" STATIC
    "earena.c"
    "einterpreter.c"
    "elex.c"
    "elist.c"
    "eparse.c"
    "estring.c"
    "eio.c"
    "effi.c"
)

target_compile_options("eruntime"
    PRIVATE "-fPIC"
)
