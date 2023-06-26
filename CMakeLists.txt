cmake_minimum_required(VERSION 3.0.0)

project(elang C)

add_executable(elang
    "src/main.c"
    "src/elex.c"
    "src/eparse.c"
    "src/elist.c"
    "src/einterpreter.c"
    "src/ebuiltin.c"
    "src/earena.c"
    "src/estring.c"
)

