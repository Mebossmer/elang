project("elibrary" C)

add_library("elibrary" SHARED
    "lib.c"
)

target_link_libraries("elibrary" "eruntime")
target_include_directories("elibrary"
    PRIVATE ${CMAKE_SOURCE_DIR}/eruntime
)

target_compile_options("elibrary"
    PRIVATE "-fPIC"
    PRIVATE "-flto"
)
