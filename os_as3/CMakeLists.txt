cmake_minimum_required(VERSION 3.26)
project(os_as3 C)

set(CMAKE_C_STANDARD 11)

add_executable(os_as3 main.c
        simplesched.c
        filelocks.c
        filelocks.h
        texts.c
        texts.h
        history.c
        history.h
        subsched.c
        subsched.h
        custerrors.c
        custerrors.h)

target_link_libraries(os_as3 m)