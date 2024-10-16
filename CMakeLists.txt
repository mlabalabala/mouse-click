cmake_minimum_required(VERSION 3.28)
project(mouse)

set(CMAKE_CXX_STANDARD 11)

add_executable(mouse main.cpp
        ini.c
        ini.h
        ConfigParse.cpp
        ConfigParse.cpp
        ConfigParse.h
        resource.h
        resource.rc
)
