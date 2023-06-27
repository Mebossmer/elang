project("ecli")

add_executable("ecli"
    "main.c"
)

target_link_libraries("ecli" "eruntime")
target_include_directories("ecli"
    PRIVATE ${CMAKE_SOURCE_DIR}/eruntime
)
