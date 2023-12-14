cmake_minimum_required(VERSION 3.26)
project(llp1 C)

set(CMAKE_C_STANDARD 11)

add_executable(llp1 src/main.c
        src/lib/query.c
        src/lib/db.c
        src/lib/graph_data.c
        src/lib/file.c)
