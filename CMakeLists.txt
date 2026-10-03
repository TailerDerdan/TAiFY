cmake_minimum_required(VERSION 4.2)
project(taify)

set(CMAKE_CXX_STANDARD 20)

add_executable(taify main.cpp
        MeelyMachine.h
        MooreMachine.h
        types.h
        Trim.h)
