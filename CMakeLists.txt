cmake_minimum_required(VERSION 3.24)

project(queuing-system C)

add_executable(main)

add_subdirectory(system)

target_sources(main
    PRIVATE
        main.c
)

target_link_libraries(main
    PRIVATE
        m
        system
)