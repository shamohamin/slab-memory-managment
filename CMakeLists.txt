cmake_minimum_required(VERSION "3.17.1")

project(slab)

add_executable(
    ${PROJECT_NAME}
    main.c
    src/slab.c
)